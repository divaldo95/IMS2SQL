//
//  IMS2SQL.cpp
//  
//
//  Created by Baranyai David on 2020. 07. 20..
//

#include <stdio.h>
#include <iostream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "SQLGenerator.hpp"
#include "IMSHandler.hpp"
#include "FileHandler.hpp"
#include "LOG.hpp"

void print_usage()
{
    std::cout << "Use -f to specify a file to convert." << std::endl;
    std::cout << "Use -d to specify a directory to convert all IMS files from it." << std::endl;
    std::cout << "-f and -d can be combined and can be used multiple times to specify more than one file or directory." << std::endl;
    std::cout << "Use -R to enable recursive directory scan. Must specified before adding any directory." << std::endl;
    std::cout << "Use -n database_name to specify the database name. By default it will use Decay." << std::endl;
    std::cout << "Use -v number to set the Valid flag explicitly. (0 = false, any other = true)" << std::endl;
    std::cout << "Use -N to specify the user's name" << std::endl;
}

int main(int argc, char** argv)
{
    bool requireConverterName = false;
    DLog::LOG &log = DLog::LOG::GetInstance();
    FileHandler fHandler;
    SQLGenerator sqlGen;
    IMSHandler ims;
    int c, index;
    while((c = getopt(argc, argv, "f:d:n:v:hRN")) != -1)
    {
        switch (c)
        {
        case 'n':
            sqlGen.SetDatabaseName(std::string(optarg));
            break;
        case 'f':
            fHandler.AppendFile(std::string(optarg));
            break;
        case 'd':
            fHandler.AppendDirectory(std::string(optarg));
            break;
        case 'h':
            print_usage();
            break;
        case 'v':
            ims.SetValidFlag(((std::stoi(optarg) == 0) ? false : true));
            break;
        case 'R':
            fHandler.EnableRecursive();
            break;
        case 'N':
            requireConverterName = true;
            break;
        default:
            abort();
        }
    }
    for (index = optind; index < argc; index++) printf ("Non-option argument %s\n", argv[index]);
    const std::vector<std::filesystem::path>& fileList = fHandler.GetFileList();

    if(requireConverterName)
    {
        std::string firstname, lastname;
        std::cout << "Enter your first name: " << std::endl;
        std::cin >> firstname;
        std::cout << "Enter your last name: " << std::endl;
        std::cin >> lastname;
        sqlGen.SetConverterName(firstname, lastname);
    }

    sqlGen.Connect();
    for (int i = 0; i < fileList.size(); i++)
    {
        if(ims.Open(fileList[i])) sqlGen.UploadIMSData(ims.GetIMSData());
        ims.Close();
        log.InsertSeparator();
    }
    sqlGen.Close();
    return 0;
}