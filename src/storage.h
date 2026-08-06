//Header file to save the last seen 
#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>

void initStorage();

void saveLastSeen(const String &value);

String loadLastSeen();

#endif