//
// Created by tomas on 28.3.2016.
//

#include "URL.h"
#include "../Viewer.hpp"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

void URL::processFile(std::string fileName)
{
    // Check if the argument is some type of URL
    if (std::regex_match(fileName, match, expresion))
    {
        int result = get_http_data(match[3],
                                   match[4],
                                   match[5],
                                   match[2]);

        if (result == 0)
            std::cout << "Work with downloaded file" << std::endl;

        std::cout << "RESULT OF FIRST GET_HTTP " << result << std::endl;

//        successor->processFile(fileName);
    }
    else
    {
        // If the argument is not valid URL, try if the successor
        // can handle it
        successor->processFile(fileName);
    }
}

int URL::get_http_data(std::string server, std::string path, std::string file, std::string protocol)
{

    namespace baio = boost::asio;
    using boost::asio::ip::tcp;

    try
    {
        boost::asio::io_service io_service;

        // Get a list of endpoints corresponding to the server name.
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(server, protocol);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // Try each endpoint until we successfully establish a connection.
        tcp::socket socket(io_service);
        baio::connect(socket, endpoint_iterator);

        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        baio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "GET " << path + file << " HTTP/1.0\r\n";
        request_stream << "Host: " << server << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";

        // Send the request.
        baio::write(socket, request);

        // Read the response status line. The response streambuf will automatically
        // grow to accommodate the entire line. The growth may be limited by passing
        // a maximum size to the streambuf constructor.
        baio::streambuf response;
        baio::read_until(socket, response, "\r\n");

        // Check that response is OK.
        std::istream response_stream(&response);

        std::string http_version;
        response_stream >> http_version;

        unsigned int status_code;
        response_stream >> status_code;

        std::string status_message;
        std::getline(response_stream, status_message);

        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
            std::cout << "Invalid response\n";
            return 1;
        }

        if (status_code != 200)
        {
            if (status_code == 302)
            {
                // Process the response headers, which are terminated by a blank line.
                std::string header;
                std::string pom;
                while (std::getline(response_stream, header) && header != "\r")
                {
//                    std::cout << header << "\n";
                    if (header.substr(0, 9) == "Location:")
                    {
                        pom = header.substr(10);
                        if (auto pos = pom.find("\r"))
                            pom = pom.substr(0, pos);
                        break;
                    }
                }

                // Check if the greped URL is valid
                if (std::regex_match(pom, match, expresion))
                {
                    return get_http_data(match[3],
                                         match[4],
                                         match[5],
                                         match[2]);
                }
                else
                {
                    std::cerr << "Redirectoin URL is not valid!" << std::endl;
                    return 1;
                }
            }
            else
            {
                std::cout << "Response returned with status code " << status_code << "\n";
                return 1;
            }
        }

        // Read the response headers, which are terminated by a blank line.
        baio::read_until(socket, response, "\r\n\r\n");

        // Process the response headers.
        std::string header, length;
        while (std::getline(response_stream, header) && header != "\r")
        {
//            std::cout << header << "\n";
            if (header.substr(0, 15) == "Content-Length:")
            {
                length = header.substr(16);
                if (auto pos = length.find("\r"))
                    length = length.substr(0, pos);
            }
        }
        std::cout << "\n";

        // ---------------------------------------------------------------------
        std::string filename = file + ".download";  // added extension to signalize working copy of file
        std::ofstream ofs(filename, std::ios_base::app);
        // ---------------------------------------------------------------------

        // Write whatever content we already have to output.
        if (response.size() > 0)
            ofs << &response;

        Viewer view;
        long len = std::stol(length);

        // Read until EOF, writing data to output as we go.
        boost::system::error_code error;

        view.printHorizontalLine();
        std::cout << "\n ";         // only visual purpose

        while (baio::read(socket, response, baio::transfer_at_least(1), error))
        {
            // Show the progress bar
            view.printProgBar("Downloading the file " + file, ofs.tellp(), len);
            ofs << &response;
        }
        if (error != baio::error::eof)
            throw boost::system::system_error(error);

        // ---------------------------------------------------------------------
        ofs.close();
        // ---------------------------------------------------------------------

        // After successfully download of the file check the size
        if (boost::filesystem::file_size(filename) == len)
        {
            // And remove the .download extension from the name of the file
            int rc = std::rename(filename.c_str(), file.c_str());
            if (rc)
                throw "Error renaming downloaded file " + filename + " to " + file;
            else
                return 0;
        }
        else
        {
            throw "Error downloaded file " + filename + " is not complete";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}