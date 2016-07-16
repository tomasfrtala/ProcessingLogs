//
// Created by tomas on 28.3.2016.
//

#include <termio.h>

#include "Unziped.h"
#include "../Viewer.hpp"
#include "../Log.hpp"

#include "../rowsFilteringStrategies/AllRows.hpp"
#include "../rowsFilteringStrategies/HTTPCode.hpp"
#include "../rowsFilteringStrategies/RequestMethod.hpp"
#include "../rowsFilteringStrategies/Date.hpp"

#include <boost/filesystem.hpp>

void Unziped::processFile(std::string fileName)
{
    std::string ext = boost::filesystem::extension(fileName);

    if (boost::filesystem::exists(fileName) && (ext == ".log"))
    {
        std::ifstream file(fileName, std::ios::in);

        if (!file.is_open())
        {
            std::cout << "File " << fileName << " can not open!" << std::endl;
        }
        else
        {
            std::cout << "The " << fileName << " opened successfully." << std::endl;

            Viewer view;
            unsigned short int rowCount = view.getRowsCount();

            Log theLog(&file, view);

            theLog.setDisplayRowStrategy(std::make_shared<AllRows>(&file, rowCount));

            // Black magic to prevent Linux from buffering keystrokes.
	    // http://stackoverflow.com/a/912184
	    // ----------------------------------------------------
            // Set terminal to raw mode 
            system("stty raw");

            char c = ' ';
            std::cin.get(c);

            while (c != 'q')
            {
                if (c == 'j')
                    theLog.showNextRows();
                else if (c == 'k')
                    theLog.showPrevRows();
                else if (c == 'f')
                    view.printFilterCmdMenu();
                else if (c == '0')    // Default: Show all rows without any filter
                    theLog.setDisplayRowStrategy(std::make_shared<AllRows>(&file, rowCount));
                else if (c == '1')
                {
                    view.printFilterHTTPCodeCmdMenu();

                    std::string code;
                    std::cin >> code;

                    theLog.setDisplayRowStrategy(std::make_shared<HTTPCode>(&file, rowCount, code));
                }
                else if (c == '2')  // Show rows filtered by Request method
                {
                    view.printFilterRequestMCmdMenu();

                    std::cin.get(c);
                    int opt = c - '0';

                    switch (opt)
                    {
                        case Request::POST: // POST
                            theLog.setDisplayRowStrategy(std::make_shared<RequestMethod>(&file,
                                                                                         rowCount,
                                                                                         Request::POST));
                            break;

                        case Request::GET: // GET
                            theLog.setDisplayRowStrategy(std::make_shared<RequestMethod>(&file,
                                                                                         rowCount,
                                                                                         Request::GET));
                            break;

                        case Request::HEAD: // HEAD
                            theLog.setDisplayRowStrategy(std::make_shared<RequestMethod>(&file,
                                                                                         rowCount,
                                                                                         Request::HEAD));
                            break;

                        case Request::UNKNOWN: // Unknown
                            theLog.setDisplayRowStrategy(std::make_shared<RequestMethod>(&file,
                                                                                         rowCount,
                                                                                         Request::UNKNOWN));
                            break;

                        default:
                            break;
                    }
                }
                else if (c == '3')
                {
                    view.printFilterDateCmdMenu();

                    std::string date;
                    std::cin >> date;

                    theLog.setDisplayRowStrategy(std::make_shared<Date>(&file, rowCount, date));
                }
                std::cin.get(c);
            }
            std::cout << std::endl;

	    // Reset terminal to normal "cooked" mode 
	    system("stty cooked"); 
        }
    }
    else
        std::cout << "No successor for handling " << fileName << "... ignore." << std::endl;
}
