//
// Created by tomas on 29.3.2016.
//

#ifndef PROCESSINGLOGS_THELOG_HPP
#define PROCESSINGLOGS_THELOG_HPP

#include <fstream>
#include <vector>
#include <iostream>
#include "rowsFilteringStrategies/RowInterface.hpp"

class Log
{
public:
    Log(std::ifstream *file)
    {
        positionAtLadder.push_back(0);

        file->seekg(0, std::ios::end);
        theEnd = file->tellg();
    }

    virtual ~Log()
    {
        delete prevRows;
        delete currRows;
        delete nextRows;
    }

    void setDisplayRowStrategy(RowInterface *strategy)
    {
        /*if (prevRows != nullptr)
            delete prevRows;

        if (currRows != nullptr)
            delete currRows;

        if (nextRows != nullptr)
            delete nextRows;*/

        prevRows = strategy;
        currRows = strategy;
        nextRows = strategy;
    }

    // Firstime print rows
    void printRows()
    {
        long currPos = currRows->read(positionAtLadder.back(), std::ios_base::beg);
        positionAtLadder.push_back(currPos);

        long nextPos = nextRows->read(currPos, std::ios_base::beg);
        positionAtLadder.push_back(nextPos);

        showCurrRows();
    }

    void showNextRows()
    {
        swapToNextRows();
        showCurrRows();
    }

    void showPrevRows()
    {
        swapToPrevRows();
        showCurrRows();
    }

private:
    void showCurrRows() const
    {
        std::cout << std::endl << currRows;
    }

    // Just switch the pointers.
    void swapToNextRows()
    {
        // Boundary check for not over jump the end of the file
        if (positionAtLadder.back() != theEnd)
        {
            auto temp = prevRows;
            prevRows = currRows;
            currRows = nextRows;
            nextRows = temp;

            long nextPos = nextRows->read(positionAtLadder.back(), std::ios_base::beg);
            positionAtLadder.push_back(nextPos);
        }
    }

    // Just switch the pointers.
    void swapToPrevRows()
    {
        // Boundary check for not over jump the begin of the file
        if (positionAtLadder.end()[-2] != 0)
        {
            auto temp = nextRows;
            nextRows = currRows;
            currRows = prevRows;
            prevRows = temp;

            // Climb up one setp at the ladder
            positionAtLadder.pop_back();

            // Climb up three setps only for looking
            prevRows->read(positionAtLadder.end()[-3], std::ios_base::beg);
        }
    }

    std::vector<long> positionAtLadder;
    long theEnd;

    RowInterface *prevRows = nullptr;
    RowInterface *currRows = nullptr;
    RowInterface *nextRows = nullptr;
};


#endif //PROCESSINGLOGS_THELOG_HPP
