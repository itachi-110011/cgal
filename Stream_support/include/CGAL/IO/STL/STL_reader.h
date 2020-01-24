// Copyright (c) 2015 GeometryFactory
//
// This file is part of CGAL (www.cgal.org);
//
// $URL$
// $Id$
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Andreas Fabri,
//                 Mael Rouxel-Labbé

#ifndef CGAL_IO_STL_STL_READER_H
#define CGAL_IO_STL_STL_READER_H

#include <CGAL/IO/io.h>
#include <CGAL/IO/reader_helpers.h>

#include <boost/cstdint.hpp>

#include <cctype>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace CGAL {
namespace IO {
namespace internal {

template <class PointRange, class TriangleRange, typename IndexMap>
bool read_ASCII_facet(std::istream& input,
                      PointRange& points,
                      TriangleRange& facets,
                      int& index,
                      IndexMap& index_map,
                      bool verbose = false)
{
  typedef typename PointRange::value_type Point;
  typedef typename TriangleRange::value_type Triangle;

  // Here, we have already read the word 'facet' and are looking to read till 'endfacet'

  std::string s;
  std::string vertex("vertex"),
              endfacet("endfacet");

  int count = 0;
  double x,y,z;
  Point p;
  Triangle ijk;
  IO::internal::resize(ijk, 3);

  while(input >> s)
  {
    if(s == endfacet)
    {
      if(count != 3)
      {
        if(verbose)
          std::cerr << "Error: only triangulated surfaces are supported" << std::endl;

        return false;
      }

      facets.push_back(ijk);
      return true;
    }
    else if(s == vertex)
    {
      if(count >= 3)
      {
        if(verbose)
          std::cerr << "Error: only triangulated surfaces are supported" << std::endl;

        return false;
      }

      if(!(input >> iformat(x) >> iformat(y) >> iformat(z)))
      {
        if(verbose)
          std::cerr << "Error while reading point coordinates (premature end of file)" << std::endl;

        return false;
      }
      else
      {
        IO::internal::fill_point(x, y, z, p);
        typename std::map<Point, int>::iterator iti = index_map.insert(std::make_pair(p, -1)).first;

        if(iti->second == -1)
        {
          ijk[count] = index;
          iti->second = index++;
          points.push_back(p);
        }
        else
        {
          ijk[count] = iti->second;
        }
      }

      ++count;
    }
  }

  if(verbose)
    std::cerr << "Error while reading facet (premature end of file)" << std::endl;

  return false;
}

template <class PointRange, class TriangleRange>
bool parse_ASCII_STL(std::istream& input,
                     PointRange& points,
                     TriangleRange& facets,
                     bool verbose = false)
{
  typedef typename PointRange::value_type Point;

  if(verbose)
    std::cout << "Parsing ASCII file..." << std::endl;

  if(!input.good())
    return false;

  // Here, we have already read the word 'solid'

  int index = 0;
  std::map<Point, int> index_map;

  std::string s, facet("facet"), endsolid("endsolid"), solid("solid");
  bool in_solid(false);
  while(input >> s)
  {
    if(s == solid)
    {
      if(in_solid)
        break;

      in_solid = true;
    }
    if(s == facet)
    {
      if(!read_ASCII_facet(input, points, facets, index, index_map, verbose))
        return false;
    }
    else if(s == endsolid)
    {
      in_solid = false;
    }
  }

  if(in_solid)
  {
    if(verbose)
      std::cerr << "Error while parsing ASCII file" << std::endl;

    return false;
  }

  return !input.fail();
}

template <class PointRange, class TriangleRange>
bool parse_binary_STL(std::istream& input,
                      PointRange& points,
                      TriangleRange& facets,
                      bool verbose = false)
{
  typedef typename PointRange::value_type Point;
  typedef typename TriangleRange::value_type Triangle;

  if(verbose)
    std::cout << "Parsing binary file..." << std::endl;

  // Start from the beginning again to simplify things
  input.clear();
  input.seekg(0, std::ios::beg);

  if(!input.good())
    return false;

  // Discard the first 80 chars (unused header)
  int pos = 0;
  char c;

  if(verbose)
    std::cout << "header: ";

  while(pos < 80)
  {
    input.read(reinterpret_cast<char*>(&c), sizeof(c));
    if(!input.good())
      break;

    if(verbose)
      std::cout << c;

    ++pos;
  }

  if(verbose)
    std::cout << std::endl;

  if(pos != 80)
    return true; // empty file

  int index = 0;
  std::map<Point, int> index_map;

  boost::uint32_t N32;
  if(!(input.read(reinterpret_cast<char*>(&N32), sizeof(N32))))
  {
    if(verbose)
      std::cerr << "Error while reading number of facets" << std::endl;

    return false;
  }

  unsigned int N = N32;
  if(verbose)
    std::cout << N << " facets to read" << std::endl;

  for(unsigned int i=0; i<N; ++i)
  {
    float normal[3];
    if(!(input.read(reinterpret_cast<char*>(&normal[0]), sizeof(normal[0]))) ||
       !(input.read(reinterpret_cast<char*>(&normal[1]), sizeof(normal[1]))) ||
       !(input.read(reinterpret_cast<char*>(&normal[2]), sizeof(normal[2]))))
    {
      if(verbose)
        std::cerr << "Error while reading normal coordinates (premature end of file)" << std::endl;

      return false;
    }

    Triangle ijk;
    IO::internal::resize(ijk, 3);

    for(int j=0; j<3; ++j)
    {
      float x,y,z;
      if(!(input.read(reinterpret_cast<char*>(&x), sizeof(x))) ||
         !(input.read(reinterpret_cast<char*>(&y), sizeof(y))) ||
         !(input.read(reinterpret_cast<char*>(&z), sizeof(z))))
      {
        if(verbose)
          std::cerr << "Error while reading vertex coordinates (premature end of file)" << std::endl;

        return false;
      }

      Point p;
      IO::internal::fill_point(x, y, z, p);

      typename std::map<Point, int>::iterator iti = index_map.insert(std::make_pair(p, -1)).first;

      if(iti->second == -1)
      {
        ijk[j] = index;
        iti->second = index++;
        points.push_back(p);
      }
      else
      {
        ijk[j] = iti->second;
      }
    }

    facets.push_back(ijk);

    // Read so-called attribute byte count and ignore it
    char c;
    if(!(input.read(reinterpret_cast<char*>(&c), sizeof(c))) ||
       !(input.read(reinterpret_cast<char*>(&c), sizeof(c))))
    {
      if(verbose)
        std::cerr << "Error while reading attribute byte count (premature end of file)" << std::endl;

      return false;
    }
  }

  return !input.fail();
}

} // namespace internal
} // namespace IO
} // namespace CGAL

#endif // CGAL_IO_STL_STL_READER_H
