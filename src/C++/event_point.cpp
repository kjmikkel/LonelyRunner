#include "event_point.h"



bool event_type::operator < (event_type* compareNode) {
  if (getHeuristic() < compareNode.getHeuristic()) {
    return true;
  }
  else {
    return false;
  }
}
