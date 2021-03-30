@echo off
title NetworkTest
start cmd /C build client -donotexit
start cmd /C build host -donotexit
