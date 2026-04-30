#ifndef COMMANDS_H
#define COMMANDS_H

#include "Polygon.h"
#include <string>
#include <vector>
#include <iostream>

double getPolygonArea(const Polygon& pol);

void cmdArea(const std::vector<Polygon>& polygons, const std::string& arg);
void cmdMinMax(const std::vector<Polygon>& polygons, const std::string& type, const std::string& arg);
void cmdCount(const std::vector<Polygon>& polygons, const std::string& arg);
void cmdPerms(const std::vector<Polygon>& polygons, std::istream& in);
void cmdMaxSeq(const std::vector<Polygon>& polygons, std::istream& in);

void processCommands(const std::vector<Polygon>& polygons, std::istream& in, std::ostream& out);

#endif
