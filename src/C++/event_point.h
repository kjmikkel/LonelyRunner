class event_point {
  
public:
  
  event_point(uint numnber_of_runners, uint rounds, uint speed, uint runnerNumber, uint type);
  // whether we are at the start or at the end of the Zone
  uint place_zone;
  
  // The number of runners
  uint number_of_runners;
  
  // How many times we have run around the zone
  uint rounds;
  
  // The speed of the runne
  uint speed;
  
  // The number of the runner - may be used later for dokumentation
  uint runnerNumber;
  
  // whether it is a final, end or start event point
  uint type;
};
