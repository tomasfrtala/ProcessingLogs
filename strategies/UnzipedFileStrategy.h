//
// Created by tomas on 28.3.2016.
//

#ifndef PROCESSINGLOGS_UNZIPEDFILESTRATEGY_H
#define PROCESSINGLOGS_UNZIPEDFILESTRATEGY_H

#include <iostream>
#include <fstream>
#include "../StrategyInterface.hpp"

class UnzipedFileStrategy: public StrategyInterface
{
public:
    UnzipedFileStrategy(std::string fileName)
        : fin(fileName, std::ios::in)
    {
    }

    void execute();

    long getCurrentPos() const
    {
        return currentPos;
    }

private:
    std::ifstream fin;
    long currentPos;
};


#endif //PROCESSINGLOGS_UNZIPEDFILESTRATEGY_H
