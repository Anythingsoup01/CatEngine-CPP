#!/bin/bash

## Clone problematic repos

exec git clone https://github.com/Anythingsoup01/ImGuizmo vendor/ImGuizmo

mkdir build; cd build; cmake ..; cmake --build . -j 10;
