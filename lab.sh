#!/bin/bash
g++    -c    main.cpp
g++    -o    lab    main.o        -lpthread
g++    -c    main2.cpp
g++    -o    lab2    main2.o        -lpthread