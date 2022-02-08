/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  Dimemas                                  *
 *       Simulation tool for the parametric analysis of the behaviour of     *
 *       message-passing applications on a configurable parallel platform    *
 *                                                                           *
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
 \*****************************************************************************/

#include <algorithm>
#include <iostream>
#include <iterator>
#include "progress.h"

using namespace std;

constexpr unsigned int STEPS_LIMIT = 1000;

class ProgressManager
{
  private:
    static ProgressManager *instance;
    ProgressManager();

    double currentProgress;
    unsigned int countSteps;

  public:
    ~ProgressManager();

    static ProgressManager *getInstance();

    bool updateProgress( double newProgress );
    void printProgress();
};

ProgressManager *ProgressManager::instance = nullptr;

ProgressManager::ProgressManager() : currentProgress( 0.0 ), countSteps( 0 )
{
}

ProgressManager::~ProgressManager()
{
}

ProgressManager *ProgressManager::getInstance()
{
  if( instance == nullptr )
    instance = new ProgressManager();

  return instance;
}

bool ProgressManager::updateProgress( double newProgress )
{
  if( newProgress > currentProgress )
  {
    currentProgress = newProgress;
    return true;
  }
  return false;
}

void ProgressManager::printProgress()
{
  if( ++countSteps % STEPS_LIMIT == 0 || currentProgress == 1.0 )
  {
    if( countSteps > 1 ) std::fill_n( std::ostream_iterator<char>( std::cout ), 6, '\b' );

    std::cout << std::fixed;
    std::cout.precision( 1 );
    std::cout.width( 5 );
    std::cout << currentProgress * 100 << "%";
    if( currentProgress == 1.0 ) 
      std::cout << "\n";
    else
      std::cout.flush();
  }
}

void updateProgress( double newProgress )
{
  if ( ProgressManager::getInstance()->updateProgress( newProgress ) )
    ProgressManager::getInstance()->printProgress();
}
