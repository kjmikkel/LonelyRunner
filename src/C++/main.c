#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>
#include <sys/time.h>
#include <vector>
#include <regex.h>
#include <json/json.h>
#include <sstream>

#include "Geometric.h"
#include "Numerical.h"
#include "data_structure.h"
#include "util.h"

struct options_data {
  int* speed_array;
  int length;
  GtkWidget* check_solution;
  GtkWidget* maximum_solution;
  GtkWidget* geo;
};

struct range_data {
  const gchar* range_from;
  const gchar* range_to;
  const gchar* num_runners;

  GtkToggleButton* pre_test;
  GtkToggleButton* geo;
  GtkLabel* label;
};

GtkWidget* make_dialog(std::string title, std::string label_text) {
  GtkWidget* dialog = gtk_dialog_new_with_buttons (title.c_str(),
						   NULL,
						   GTK_DIALOG_MODAL,
						   GTK_STOCK_OK, 
						   GTK_RESPONSE_ACCEPT,
						   NULL);
        
      GtkWidget* label = gtk_label_new((const char*)label_text.c_str());
      gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE, TRUE, 0);
      gtk_widget_show (label);
      return dialog;
}

static bool checkForSolution(int speedArray[], int length) {
  
  for(int numberIndex = 2; numberIndex < length + 2; numberIndex++) {
    // get a number in the the set {2, ..., n+1}
    int number = numberIndex;
    
    bool doesDevide = false;
    
    for(int speedIndex = 0; speedIndex < length; speedIndex++) {
      doesDevide |= (speedArray[speedIndex] % number == 0);
      
      // If the number does devide one of the speeds, then we can move on to the next number
      if(doesDevide) {
	break;
      }
    }

    /* 
     * If we arrive at this point, and the number does not devide, then there exists at least one number the that cannot be devided with another number,
     * and in that case the given configuration holds for the (1) equation/criterion. 
     */
    if (!doesDevide) {
      return true;
    }
  }
  
  return false;
}

std::string intToString(int i)
{
    std::stringstream ss;
    std::string s;
    ss << i;
    s = ss.str();
    return s;
}
/*
int stringToInt(std::string str) {
  istringstream buffer(str);
  int value;
  buffer >> value;
  return value;
}
*/

void recursive_array(int* array, int* number_array,
		     int level, int array_index,
		     int max_number,
		     bool pre_test, bool geo,
		     GtkLabel* label) {

  for(int current_index = array_index - level; current_index < max_number; current_index++) { 
    array[array_index - level] = number_array[current_index];
    
    if (level < array_index) {
      if (level == 1) {
	std::string testing = "Now testing index " + intToString(array[array_index - level]);
	gtk_label_set_text(label, testing.c_str());
	printf("Now testing index %d\n", array[array_index - level]);
      }
      
    recursive_array(array, number_array, 
		    level + 1, array_index,
		    current_index,
		    pre_test, geo,
		    label);
    
    } else {
      /*
      for(int index = 0; index < array_index; index++) {
	std::cout << " " << array[index];
      }
      std::cout << "\n";
      */
      
      if (pre_test) {
	// If it is true, then we might as well return
	if(checkForSolution(array, array_index)) {
	  return;
	      }
      }

      if (geo) {
      geo_time_result* geo_result = Geometric_method(array,		     
					   	     array_index);
      
      // We check for errors
      if(geo_result != NULL && (!geo_result->result || !isValid(geo_result, array))) {
	  
	printf("error: %d, %d\n", geo_result->result, isValid(geo_result, array));
	printf("For the values: [");
	
	for(int index = 0; index < array_index; index++) {  
	  printf(", %d", array[index]);
	}
	printf("]\n\n");
      }
      delete geo_result;
      } else {
	// Test that what it reports
	num_time_result* num_result = Numerical_method(array, array_index, false, false, false);
	delete num_result;
      }
    }
  }
}

// This will test every single possible combination of speeds under or equal to 100 with 10 runners 
void range_test(GtkWidget *wid, range_data* range) {

  int start_value = atoi(range->range_from);
  int end_value = atoi(range->range_to);
  int num_runners = atoi(range->num_runners);

  bool pre_test = (range->pre_test)->active;
  bool geo = (range->geo)->active;
  
  std::cout << start_value << " " << end_value << " " << num_runners << "\n";
  // Add error msg
  if (start_value < 1) {
    return;
  }

  // Add error msg
  if (end_value <= start_value) {
    return;
  }

  // Add error msg
  if (num_runners > (end_value - start_value)) {
    return;
  }
  struct timeval start;
  struct timeval end;
  
  struct timezone tz;
  struct tm *tm;

  // The array which are going to contain all the different permutations of speeds below 100
  int test_array[num_runners];

  // The array that is going to contain the values we are going to check
  int max_number = end_value - start_value;
  int real_number_array[max_number];
  
  for(int index = 0; index < max_number; index++) {
    real_number_array[index] = index + start_value;
  }

  // Now to populate the array with the values and test them
  gettimeofday(&start, &tz);
  
  recursive_array(test_array, real_number_array, 
		  1, num_runners,
		  max_number,
		  pre_test, geo,
		  range->label);
  
  gettimeofday(&end, &tz);
  std::cout << "\nDone. Making this took ";
  
  std::stringstream ss;
  ss << (end.tv_usec - start.tv_usec + (end.tv_sec - start.tv_sec) * 1000000);
  std::cout << ss.str() << " microseconds.\n";
  
  // Add pop-up

  delete tm;
}

static void range_test_widget(GtkWidget *wid, GtkWidget *widget) {
  GtkWidget* win = NULL;
  GtkWidget* vbox = NULL;
  GtkWidget* hbox = NULL;

  GtkWidget* from_label = NULL;
  GtkWidget* to_label = NULL;
  GtkWidget* num_runners_label= NULL;
  GtkWidget* status_label = NULL;

  GtkWidget* from_entry = NULL;
  GtkWidget* to_entry = NULL;
  GtkWidget* num_runners_entry = NULL;

  GtkWidget* check_widget = NULL;
  GtkWidget* check_maximum = NULL;
  GtkWidget* geo_radio = NULL;
  GtkWidget* num_radio = NULL;
  
  GtkWidget* ok_button = NULL;
  GtkWidget* cancel_button = NULL;

  range_data* range = new range_data;
  
  win = gtk_window_new (GTK_WINDOW_TOPLEVEL); 
  gtk_container_set_border_width (GTK_CONTAINER (win), 8);
  gtk_window_set_title (GTK_WINDOW (win), "Define Range options");
  gtk_window_set_position (GTK_WINDOW (win), GTK_WIN_POS_CENTER);
  gtk_widget_realize (win);

  vbox = gtk_vbox_new (TRUE, 3);
  gtk_container_add (GTK_CONTAINER (win), vbox);

  hbox = gtk_hbox_new(TRUE, 0);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);

  // The labels
  from_label = gtk_label_new("Start of range (>0):"); 
  gtk_box_pack_start(GTK_BOX(hbox), from_label, TRUE, TRUE, 0);

  to_label = gtk_label_new("End of range:");
  gtk_box_pack_start(GTK_BOX(hbox), to_label, TRUE, TRUE, 0);
  
  // The entry boxes
  hbox = gtk_hbox_new(TRUE, 0);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);
  
  from_entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(from_entry), "1");
  gtk_box_pack_start(GTK_BOX(hbox), from_entry, TRUE, TRUE, 0);
  range->range_from = gtk_entry_get_text(GTK_ENTRY(from_entry));

  to_entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(to_entry), "100");
  gtk_box_pack_start(GTK_BOX(hbox), to_entry , TRUE, TRUE, 0);
  range->range_to = gtk_entry_get_text(GTK_ENTRY(to_entry));

   // The check boxes
  hbox = gtk_hbox_new(TRUE, 0);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);
  num_runners_label = gtk_label_new("The number of Runners");
  gtk_box_pack_start(GTK_BOX(hbox), num_runners_label, TRUE, TRUE, 0);

  status_label = gtk_label_new("Status: Not run yet");
  gtk_box_pack_start(GTK_BOX(hbox), status_label, TRUE, TRUE, 0);
  range->label = GTK_LABEL(status_label);

  // The last entry box
  hbox = gtk_hbox_new(TRUE, 0);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);

  num_runners_entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(num_runners_entry), "1");
  gtk_box_pack_start(GTK_BOX(hbox), num_runners_entry, TRUE, TRUE, 0);
  range->num_runners = gtk_entry_get_text(GTK_ENTRY(num_runners_entry));

  check_maximum = gtk_check_button_new_with_label("Do pretest");
  gtk_box_pack_start(GTK_BOX(hbox), check_maximum, TRUE, TRUE, 0);
  gtk_widget_set_name(check_maximum, "check_solution");
  range->pre_test = GTK_TOGGLE_BUTTON(check_maximum);

  // The radio buttons
  hbox = gtk_hbox_new(TRUE, 0);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);

  geo_radio = gtk_radio_button_new_with_label(NULL, "Find a solution with the Geometrical algorithm");
  gtk_box_pack_start(GTK_BOX(hbox), geo_radio, TRUE, TRUE, 0);
  range->geo = GTK_TOGGLE_BUTTON(geo_radio);

  num_radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(geo_radio), "Find a solution with the Numerical algorithm");
  gtk_box_pack_start(GTK_BOX(hbox), num_radio, TRUE, TRUE, 0);
  
  // The buttons
  hbox = gtk_hbox_new(TRUE, 2);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);

  ok_button = gtk_button_new_from_stock ("Run range test");
  g_signal_connect (G_OBJECT (ok_button), 
		    "clicked", 
		    G_CALLBACK (range_test), 
		    (gpointer) (void*)range);
  gtk_box_pack_start (GTK_BOX (hbox), ok_button, TRUE, TRUE, 0);
  
  cancel_button = gtk_button_new_from_stock ("Cancel");
  g_signal_connect_swapped (G_OBJECT (cancel_button), 
		    "clicked", 
		    G_CALLBACK (gtk_widget_destroy), 
		    G_OBJECT (win) );
  
  gtk_box_pack_start (GTK_BOX (hbox), cancel_button, TRUE, TRUE, 0);
  
  gtk_widget_show_all (win);
  }

static void do_test(GtkWidget *wid, options_data* options) { 
  int* speed_array = options->speed_array;
  int length = options->length;
  
  std::cout << "Do test\n";
  for(int index = 0; index < length; index++) {
    if(speed_array[index] < 1) {
	std::string str_title = "***Error - illegal value:"; 
	std::string str_msg = "The value " + intToString(speed_array[index]) + " is not a legal value";   
	GtkWidget* positive_dialog = make_dialog(str_title, str_msg);
	gint result = gtk_dialog_run(GTK_DIALOG(positive_dialog));
	gtk_widget_destroy(positive_dialog);
    }
  }

  bool check_for_solution = GTK_TOGGLE_BUTTON(options->check_solution)->active;
  bool check_for_maximum_solution = GTK_TOGGLE_BUTTON(options->maximum_solution)->active;

  if (check_for_maximum_solution)
    std::cout << "Max\n";
  
  bool geo_check = GTK_TOGGLE_BUTTON(options->geo)->active;
  
  /* 
   * This is not dependent on check_for_maximum_solution, as this depends on the individual values of the  
   * numbers, not on the relation between them
   */
  if(check_for_solution) {
    std::cout << "check Solution\n";
    bool solution = checkForSolution(speed_array, length);
    if (solution)  {
      
      std::string str = "There exists a solution for the values";
      /*
      std::stringstream out;
      
      if (length <= 25) {
      str += " ";
	for(int index = 0; index < length; index++) {
	  std::cout << speed_array[index];
	  out << intToString(speed_array[index]);
	  if ((index % 10) == 0)
	    out << "\n";
	    }
	std::cout << out.str();
      }
      
      str += out.str();
      */      
      GtkWidget* dialog = make_dialog("There exists a solution", str);
      gint result = gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
      return;
    } else {
      std::string str = "Could not detect a solution up-front - will now run main algorithm"; 
      GtkWidget* dialog = make_dialog("Could not detect whether a solution exists up-front", str);
      gint result = gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
    }
    
  }
  
  geo_time_result* geo_result;
  num_time_result* num_result;

  if (geo_check) {
    std::cout << "Run Geo\n"; 
        
    geo_time_result* geo_result = Geometric_method(speed_array, length);
    std::cout << "After Geo: " << geo_result << "\n";
    
    if (geo_result->result) {
      // We found a result 
      if(isValid(geo_result, speed_array)) {
	std::cout << "Geo result\n";
	std::string str_geo_pos = "GEO: There exists a valid solution to the runner speeds"; 
	GtkWidget* positive_dialog = make_dialog(str_geo_pos, str_geo_pos);
	gint result = gtk_dialog_run(GTK_DIALOG(positive_dialog));
	gtk_widget_destroy(positive_dialog);
      } else {
	std::string str_geo_fail = "GEO: Invalid result found";
	std::string str_geo_fail2 = "Please make note of the runner speeds, send an error report, and try the Numerical algorithm";
	GtkWidget* fail_dialog = make_dialog(str_geo_fail, str_geo_fail2);
	gint result = gtk_dialog_run(GTK_DIALOG(fail_dialog));
	gtk_widget_destroy(fail_dialog);
      } 
    } else {
      // We did not find a result!
      std::string str_geo_fail = "GEO: No result found for the given runner speeds";
      std::string str_geo_fail2 = "No results found, please run this again with the numerical algorithm";
      GtkWidget* fail_dialog = make_dialog(str_geo_fail, str_geo_fail2);
      gint result = gtk_dialog_run(GTK_DIALOG(fail_dialog));
      gtk_widget_destroy(fail_dialog);
    }
    std::cout << "Delete geo\n";
    delete geo_result;

  } else {

    num_time_result* num_result = Numerical_method(speed_array, length, false, false, check_for_maximum_solution);
    if (num_result->result) {
      
      if(isValid(num_result, speed_array, length)) {
	std::string str_num_pos = "NUM: There exists a valid solution to the runner speeds"; 
	GtkWidget* positive_dialog = make_dialog(str_num_pos, str_num_pos);
	gint result = gtk_dialog_run(GTK_DIALOG(positive_dialog));
	gtk_widget_destroy(positive_dialog);
      } else {
	std::string str_geo_fail = "NUM: Invalid result found";
	std::string str_geo_fail2 = "Please make note of the runner speeds, send an error report - this should never happen";
	GtkWidget* fail_dialog = make_dialog(str_geo_fail, str_geo_fail2);
	gint result = gtk_dialog_run(GTK_DIALOG(fail_dialog));
	gtk_widget_destroy(fail_dialog);
      }
	
    } else {
      // We did not find a result!
      std::string str_num_fail = "NUM: No result found for the given runner speeds!";
      std::string str_num_fail2 = "No results found - make note of your inputs. Make sure none of them are below 1.\nIf no of the values are below 1, you may have a counter-example to the Lonely Runner conjecture.";
      GtkWidget* fail_dialog = make_dialog(str_num_fail, str_num_fail2);
      gint result = gtk_dialog_run(GTK_DIALOG(fail_dialog));
      gtk_widget_destroy(fail_dialog);
    }
    delete num_result;
  }
  
}

static void options_widget(int speed_array[], int length) {
  
  GtkWidget* win = NULL;
  GtkWidget* vbox = NULL;
  GtkWidget* hbox = NULL;
  GtkWidget* check_widget = NULL;
  GtkWidget* check_maximum = NULL;
  GtkWidget* geo_radio = NULL;
  GtkWidget* num_radio = NULL;
  
  GtkWidget* ok_button = NULL;
  GtkWidget* cancel_button = NULL;

  for(int index = 0; index < length; index++) {
    std::cout << speed_array[index] << "\n";
  }

  options_data* option = new options_data;
  option->speed_array = speed_array;
  option->length = length;
  
  std::cout << "Option_data speed:\n";
  for(int index = 0; index < length; index++) {
    std::cout << option->speed_array[index] << "\n";
  }
  std::cout << "Option data speed end\n";

  win = gtk_window_new (GTK_WINDOW_TOPLEVEL); 
  gtk_container_set_border_width (GTK_CONTAINER (win), 8);
  gtk_window_set_title (GTK_WINDOW (win), "Options");
  gtk_window_set_position (GTK_WINDOW (win), GTK_WIN_POS_CENTER);
  gtk_widget_realize (win);

  vbox = gtk_vbox_new (TRUE, 2);
  gtk_container_add (GTK_CONTAINER (win), vbox);

  hbox = gtk_hbox_new(TRUE, 0);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);

  // The check boxes
  check_maximum = gtk_check_button_new_with_label("Check whether a solution exists");
  gtk_box_pack_start(GTK_BOX(hbox), check_maximum, TRUE, TRUE, 0);
  gtk_widget_set_name(check_maximum, "check_solution");
  option->check_solution = check_maximum;

  check_widget = gtk_check_button_new_with_label("Find Maximum distance (using Numerical algorithm)");
  gtk_box_pack_start(GTK_BOX(hbox), check_widget, TRUE, TRUE, 0);
  gtk_widget_set_name(check_widget, "check_maximum_distance (This will take a while)");
  option->maximum_solution = check_widget;

  // The radio buttons
  hbox = gtk_hbox_new(TRUE, 2);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);

  geo_radio = gtk_radio_button_new_with_label(NULL, "Find a solution with the Geometrical algorithm");
  gtk_box_pack_start(GTK_BOX(hbox), geo_radio, TRUE, TRUE, 0);
  option->geo = geo_radio;

  num_radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(geo_radio), "Find a solution with the Numerical algorithm");
  gtk_box_pack_start(GTK_BOX(hbox), num_radio, TRUE, TRUE, 0);
  
  // The buttons
  hbox = gtk_hbox_new(TRUE, 2);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);

  ok_button = gtk_button_new_from_stock ("Run");
  g_signal_connect (G_OBJECT (ok_button), 
		    "clicked", 
		    G_CALLBACK (do_test), 
		    (gpointer) (void*)option);
  gtk_box_pack_start (GTK_BOX (hbox), ok_button, TRUE, TRUE, 0);
  
  cancel_button = gtk_button_new_from_stock ("Cancel");
  g_signal_connect_swapped (G_OBJECT (cancel_button), 
		    "clicked", 
		    G_CALLBACK (gtk_widget_destroy), 
		    G_OBJECT (win) );
  
  gtk_box_pack_start (GTK_BOX (hbox), cancel_button, TRUE, TRUE, 0);
  
  gtk_widget_show_all (win);
}

static void import(GtkWidget *wid, GtkWidget *win) {
  GtkWidget* diag;
  diag = gtk_file_chooser_dialog_new ("Open File",
				      NULL, //parent_window,
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);
  
  if (gtk_dialog_run (GTK_DIALOG (diag)) == GTK_RESPONSE_ACCEPT)
    {
      char* filename;
      
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (diag));
     
      len_array l_arr = read_json_file_array(filename);
      delete filename;
            
      options_widget(l_arr.array, l_arr.len);
      //      delete l_arr.array;
  }
  gtk_widget_destroy (diag);

}

static void testValues(int startRunners, 
		       int endRunners, 
		       int runnerDelta, 
		       int startArgNum, 
		       int endArgNum, 
		       int argNumDelta, 
		       bool primevise) {
  
  for(int offset = startArgNum; offset < endArgNum; offset += argNumDelta) {

    for(int runnerNum = startRunners; runnerNum < endRunners; runnerNum += runnerDelta) {

      int array[runnerNum];      
      for(int index = 0; index < runnerNum; index++) {
	array[index] = index + offset;
      }
      
      timeval* start = new timeval;
      timeval* end = new timeval;
      gettimeofday(start, NULL);
      geo_time_result* geo_result = Geometric_method(array, runnerNum);
      gettimeofday(end, NULL);
      
      bool valid = isValid(geo_result, array); 
      
      event_point point = geo_result->point;
      float f_geo_result = float(point.local_position + point.rounds * (point.number_of_runners + 1)) / float(point.speed * (point.number_of_runners + 1.0));
      printf("Time: %f, Valid: %i\n", f_geo_result, valid);
            
      gettimeofday(start, NULL);
      num_time_result* num_result = Numerical_method(array, runnerNum, true, false, false);
      gettimeofday(end, NULL);
      
      valid = isValid(num_result, array, runnerNum);
      float f_num_result = float(num_result->a) / float(num_result->k1 + num_result->k2);
    
      printf("Time: %f, Valid: %d", f_num_result, valid);
      
      delete geo_result;
      delete num_result;
      delete end;
      delete start;
    }	
  } 
  
}

static void testCustom() {
  
  int length = 1000;
  int array[length];

  for(int index = 0; index < length; index++) {
    array[index] = index + 30050;
  }
  
  
  timeval* start = new timeval;
  timeval* end = new timeval;
  gettimeofday(start, NULL);
  geo_time_result* geo_result = Geometric_method(array, length);
  gettimeofday(end, NULL);
  
  bool valid = isValid(geo_result, array); 
  bool print = true;
  
  bool solution = checkForSolution(array, length);

  printf("Can we be sure there is a solution?: %i\n", solution);
  
  
  event_point point = geo_result->point;
  float f_geo_result = float(point.local_position + point.rounds * (point.number_of_runners + 1)) / float(point.speed * (point.number_of_runners + 1.0));
  printf("Time: %f, Valid: %i\n", f_geo_result, valid);
  
  gettimeofday(start, NULL);
  num_time_result* num_result = Numerical_method(array, length, true, false, false);
  gettimeofday(end, NULL);
  
  valid = isValid(num_result, array, length);
  float f_num_result = float(num_result->a) / float(num_result->k1 + num_result->k2);
  
  delete geo_result;
  delete num_result;
  delete end;
  delete start;
  
}

void parse_text_field(GtkWidget *wid, GtkWidget *widget) {
  
  GtkEntry* entry = (GtkEntry*)widget;
  
  char* temp_val = (char*)gtk_entry_get_text(entry);
  char val[strlen(temp_val)];
  strcpy(val, temp_val);
  char* test;
  
  std::vector<std::string> vec;
  std::string delimiter = " ,";

  test = strtok(val, delimiter.c_str());
  while(test != NULL) {
    std::cout << "Token: " << test  << "\n";
    vec.push_back(test);
    test = strtok(NULL, delimiter.c_str());
  }

  int length = vec.size();
  int* runner_array = new int[length];
  bool fail = false;

  for(int index = 0; 0 < vec.size(); index++) {
    
    int candidate =  atoi(vec.back().c_str());
    std::cout << "Num: " << candidate << "\n";
    vec.pop_back();
    if (candidate > 0) {
      
      runner_array[index] = candidate;
    } else {
      // An illegal value has been detected, stoping procedure - inform the user of the error
      fail = true;
      break;
    }
  }

  if (!fail) {
    options_widget(runner_array, length);
  } else {
    std::string str = "Cannot parse text field - illegal input detected";
      
    GtkWidget* dialog = gtk_dialog_new_with_buttons (str.c_str(),
						     NULL,
						     GTK_DIALOG_MODAL,
						     GTK_STOCK_OK, 
						     GTK_RESPONSE_ACCEPT,
						     NULL);
    
    
    GtkWidget* label = gtk_label_new((const char*)str.c_str());
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE, TRUE, 0);
    gtk_widget_show (label);
    
    gint result = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    return;
  }


}

int main (int argc, char *argv[])
{
  
  GtkWidget* button = NULL;
  GtkWidget* win = NULL;
  GtkWidget* vbox = NULL;
  GtkWidget* label = NULL;
  GtkWidget* entry = NULL;
  GtkWidget* option = NULL;

  /* Initialize GTK+ */
  
  g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
  gtk_init (&argc, &argv);
  g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);
  
  /* Create the main window */
  
  win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (win), 8);
  gtk_window_set_title (GTK_WINDOW (win), "Lonely Runner Verifier");
  gtk_window_set_position (GTK_WINDOW (win), GTK_WIN_POS_CENTER);
  gtk_widget_realize (win);
  g_signal_connect (win, "destroy", gtk_main_quit, NULL);
  
  /* Create a vertical box with buttons */
  
  vbox = gtk_vbox_new (TRUE, 6);
  gtk_container_add (GTK_CONTAINER (win), vbox);
  
  label = gtk_label_new("Enter the speeds of your Runners (comma seperated)");
  gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);

  entry = gtk_entry_new ();
  gtk_widget_show(entry);
  gtk_box_pack_start(GTK_BOX(vbox), entry, TRUE, TRUE, 0);
  
  button = gtk_button_new_from_stock ("Run Test");
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (parse_text_field), (gpointer) entry);
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock ("Import speeds to test");
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (import), (gpointer) win);
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock ("Make range test");
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (range_test_widget), (gpointer) win);
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  g_signal_connect (button, "clicked", gtk_main_quit, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);
  
  // testCustom();

  /* Enter the main loop */
  gtk_widget_show_all (win);
  gtk_main ();
  return 0;
}

