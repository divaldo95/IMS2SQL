/*
 * DecayToolTest.cpp
 * Created by David Baranyai
 * 2020.11.20
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "DecayTool.hpp"

int main(int argc, char** argv)
{
    DecayTool decay;
    decay.Connect("username", "password", "database"); //first you need to connect to the database
    std::string filename = decay.GetFileNamebyQCID(1); //Get file name by qcid.
    decay.SetQCValidFlag(1, false); //Set Valid flag to false for ID 1
    decay.Close(); //Not necessary, but not an issue
}