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
#include <NTL/ZZ.h>
#include <NTL/RR.h>
#include <pthread.h>
#include "boost/thread.hpp"


#include "Geometric.h"
#include "Numerical.h"
#include "data_structure.h"
#include "util.h"

NTL_CLIENT

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
  const gchar* start_val;

  GtkToggleButton* pre_test;
  GtkToggleButton* geo;
  GtkLabel* label;
};


struct range_data_thread {
  // The array with the configuration
  int* array;

  // The array containing the range of numbers
  const int* number_array;

  // The worklist, which contains the work that a given thread must do
  const int* worklist_array;
  int worklist_array_length;

  // How many runners there are
  int array_index;

  int max_number;
  int* error;
  bool pre_test;
  bool geo;
  GtkLabel* label;
};

//pthread_t* threads[numCPU];

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

/*
int stringToInt(std::string str) {
  istringstream buffer(str);
  int value;
  buffer >> value;
  return value;
}
*/

void report_possible_invalid(int* array_speed, int length) {
  // Show to user
  std::string array_values = "";
  // json output
  std::string array_out = "[";
  // small optimzation
  std::string str_int = "";

  for(int index = 0; index < length; index++) {
    str_int = intToString(array_speed[index]);
    array_values += str_int + " ";
    if((index % 50 == 0) && (index > 1) ) {
      array_values += "\n";
    }


    array_out += "\"" + str_int + "\"";
    if (index + 1 < length) {
      array_out += ", ";
    }
  }
  array_out += "]";

  std::string str_title = "Possible invalid configurations of speeds found";
  std::string str_msg = "The following values are possibly invalid: " + array_values + "\nPlease test with both algorithms in order to verify the result.\nYou will now get the chance to save the current array to a file (in the json data-format) for later retrival.";

  cout << "print stuff to screen\n" << array_out;

  GtkWidget* error_dialog = make_dialog(str_title, str_msg);
  gint result = gtk_dialog_run(GTK_DIALOG(error_dialog));
  gtk_widget_destroy(error_dialog);

    GtkWidget* diag;
    diag = gtk_file_chooser_dialog_new ("Save File",
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					NULL);

    if (gtk_dialog_run (GTK_DIALOG (diag)) == GTK_RESPONSE_ACCEPT)
      {
	char* filename;

	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (diag));
	write_to_file(filename, array_out);

	delete filename;
      }
    gtk_widget_destroy (diag);
}

int recursive_array(int* array, const int* number_array,
		     int level, int array_index,
		     int max_number,
		     bool pre_test, bool geo,
		     GtkLabel* label) {


	for(int current_index = array_index - level; current_index < max_number; current_index++) {
	  array[array_index - level] = number_array[current_index];

	  // Are we still recursing out way down?
	  if (level < array_index) {

	    // yes, then lets go on
	    int run = recursive_array(array, number_array,
				      level + 1, array_index,
				      current_index,
				      pre_test, geo,
				      label);

	  // If we return the error msg 1, then we stop the entire system,
	  if (run == 1) {
	    return 1;
	  }

	} else {
	  // Otherwise, lets do the test
/*
	    for(int index = 0; index < array_index; index++) {

	      std::cout << " " << array[index];
	    }
	    std::cout << "\n";
*/

	  if (pre_test) {
	    // If it is true, then we might as well return
	    if(checkForSolution(array, array_index)) {
	      continue;
	    }
	  }

	  if (geo) {
	    geo_time_result* geo_result = Geometric_method(array,
							   array_index);

	    // We check for errors
	    if(geo_result == NULL) {
	      std::string str_title = "Geo solution could not be found";
	      std::string str_msg = "There could not be found a geo solution - there is something very wrong with your settings";
	      GtkWidget* error_dialog = make_dialog(str_title, str_msg);
	      gint result = gtk_dialog_run(GTK_DIALOG(error_dialog));
	      gtk_widget_destroy(error_dialog);
	      return 1;
	    }

	    bool geo_bool_result = !geo_result->result;
	    bool geo_bool_check_result = !isValid(geo_result, array);

	    if(geo_bool_result || geo_bool_check_result) {

	      std::cout << "Geo error: " << geo_bool_result << ", " << geo_bool_check_result << "\n";

	      cout << "done\n";
	      report_possible_invalid(array, array_index);
	      delete geo_result;
	      return 1;
	    }
            
	    delete geo_result;

	  } else {
	    /*
	      for(int index = 0; index < array_index; index++) {
	      std::cout << " " << array[index];
	      }
	    */
	    num_time_result* num_result = Numerical_method(array, array_index, false, false, false);

	    bool num_bool_result = !num_result->result;
	    bool num_bool_check_result = !isValid(num_result, array, array_index);

	    if(num_bool_result || num_bool_check_result) {
	      std::cout << "Num error: " << num_bool_result << ", " << num_bool_check_result << "\n";

	      report_possible_invalid(array, array_index);
	      delete num_result;
	      return 1;
	    }

	    delete num_result;
	  }
	}
      }
      return 0;
}

void* thread_call(void* data) {
  range_data_thread* thread_data = (range_data_thread*)data;

  const int* worklist_array = thread_data->worklist_array;
  int worklist_length = thread_data->worklist_array_length;

  int* array = thread_data->array;
  const int* number_array = thread_data->number_array;

  int array_index = thread_data->array_index;
  int* error = thread_data->error;
  bool pre_test = thread_data->pre_test;
  bool geo = thread_data->geo;
  GtkLabel* label = thread_data->label;

  for(int index = 0; index < worklist_length; index++) {

    array[array_index - 2] = worklist_array[index];
    cout << "Second index: " << array[array_index - 2] << ", source says: " << worklist_array[index] << "\n";
	  int run = recursive_array(array, number_array,
	  3, array_index,
	  array[array_index - 2] - 1,
	  pre_test, geo,
	  label);

          if (run == 1)
              break;
  }
}

//boost::thread make_thread();

int second_level(int* array,const int* number_array,
		     int level, int array_index,
		     int max_number,
		     bool pre_test, bool geo,
		     GtkLabel* label) {

  int numCPU = sysconf( _SC_NPROCESSORS_ONLN );
  int difference = max_number - array_index;

  // if we are at level 2, then we begin to use the multiple cores, but only if we have them
  if (array_index == 1 || numCPU == 1 || difference < 2) {
    return recursive_array(array, number_array, level, array_index, max_number, pre_test, geo, label);
} else {

    boost::thread_group group;

    int foreach = (difference + 1) / numCPU;
    int mod = (difference + 1) % numCPU;

    //   printf("foreach: %d, mod: %d numCPU: %d\n", foreach, mod, numCPU)
    int startIndex = foreach + mod;
    int* worklist_array = new int[startIndex];
    memcpy(worklist_array, number_array + array_index - 1, startIndex * sizeof(int));

    int* thread_array = new int[array_index];
    memcpy(thread_array, array, array_index * sizeof(int));

    int error = 0;

    // The first thread 
    for(int i = 0; i < startIndex; i++) {
        cout << "Thread: 1, Num " << worklist_array[i] << ", at index: " << i  << "\n";
    }

    range_data_thread* outer_thread_data = new range_data_thread;
    outer_thread_data->worklist_array = worklist_array;
    outer_thread_data->worklist_array_length = startIndex;
    outer_thread_data->array = thread_array;
    outer_thread_data->number_array = number_array;
    outer_thread_data->array_index = array_index;
    outer_thread_data->error = &error;
    outer_thread_data->pre_test = pre_test;
    outer_thread_data->geo = geo;
    outer_thread_data->label = label;

    group.create_thread(boost::bind(thread_call, outer_thread_data));

    // All the remaining threads
    for(int index = 1; index < numCPU; index++) {

      // Make the new arrays and populate it
      worklist_array = new int[foreach];
      memcpy(worklist_array,
             number_array + (array_index - 2) + startIndex + foreach * (index - 1),
             foreach * sizeof(int));

      cout << "index: " << (array_index - 2) + startIndex + foreach * (index - 1) << "\n";
      for(int i = 0; i < foreach; i++) {
        cout << "Thread: " << (index + 1) << ", Num " << worklist_array[i] << ", at index: " << i  << "\n";
    }

      thread_array = new int[array_index];
      memcpy(thread_array,
             array,
             array_index * sizeof(int));


      range_data_thread* thread_data = new range_data_thread;
      thread_data->worklist_array = worklist_array;
      thread_data->worklist_array_length = foreach;
      
      thread_data->array = thread_array;
      thread_data->number_array = number_array;
      thread_data->array_index = array_index;
      thread_data->error = &error;
      thread_data->pre_test = pre_test;
      thread_data->geo = geo;
      thread_data->label = label;

      group.create_thread(boost::bind(thread_call, thread_data));
    }
    
    group.join_all();

    return error;

  }
}

void range_test(int start_value, int end_value, int num_runners, int start_max_val, bool pre_test, bool geo, GtkLabel* label) {
  struct timeval start;
  struct timeval end;

  struct timezone tz;
  struct tm *tm;

  if (start_max_val < start_value + num_runners - 1) {
    start_max_val = start_value + num_runners - 1;
  } else if (start_max_val > end_value) {
    start_max_val = end_value;
  }

  // The array which are going to contain all the different permutations of speeds below 100
  std::cout << "num runners: " << num_runners << "\n";
 
   // The array that will contain the speeds we are going to check
  int test_array[num_runners];

  // The array that contains the range of numbers we are going to test
  int max_number = end_value - start_value;
  int real_number_array[max_number];

  // We populate the array with the number range we are going to test, and find 
  // the start index
  int first_level_index = 0;

  for(int index = 0; index <= max_number; index++) {
    real_number_array[index] = index + start_value;
    if (index + start_value == start_max_val) {
      first_level_index = index;
    }
  }



  // Now to populate the array for the runner speeds and tests them
  gettimeofday(&start, &tz);
  int run = 0;

  for(int index = first_level_index; index < max_number; index++) {

    test_array[num_runners - 1] = real_number_array[index];
    std::string testing = "Now testing index " + intToString(test_array[num_runners - 1]);
    if (label != NULL) {
      gtk_label_set_text(label, testing.c_str());
    }
    printf("Now testing index %d\n", test_array[num_runners - 1]);

    run = second_level(test_array, real_number_array,
			  2, num_runners,
			  index,
			  pre_test, geo,
			  label);

    if (run == 1) {
      break;
    }
  }
  gettimeofday(&end, &tz);
  std::cout << "\nDone. Making this took ";

  std::stringstream ss;
  ss << (end.tv_usec - start.tv_usec + (end.tv_sec - start.tv_sec) * 1000000);
  std::cout << ss.str() << " microseconds.\n";
  if (run == 1) {
    std::cout << "Stopped due to possible invalid configuration\n";
  }
  // Add pop-up

  delete tm;
}

// This will test every single possible combination of speeds under or equal to 100 with 10 runners
void range_test_entrence(GtkWidget *wid, range_data* range) {

  int start_value = stringToInt(range->range_from);
  int end_value = stringToInt(range->range_to);
  int num_runners = stringToInt(range->num_runners);
  int start_max_value = stringToInt(range->start_val);
  bool pre_test = (range->pre_test)->active;
  bool geo = (range->geo)->active;

  GtkLabel* label = range->label;
  // Relese the data
  //  delete range;

  if (start_value < 1) {
    std::string str_title = "Invalid start value";
    std::string str_msg = "The start value must be greater than 0";
    GtkWidget* error_dialog = make_dialog(str_title, str_msg);
    gint result = gtk_dialog_run(GTK_DIALOG(error_dialog));
    gtk_widget_destroy(error_dialog);

    return;
  }

  if (end_value <= start_value) {
    std::string str_title = "Invalid end range value";
    std::string str_msg = "The end tange value must be greater than the start value";
    GtkWidget* error_dialog = make_dialog(str_title, str_msg);
    gint result = gtk_dialog_run(GTK_DIALOG(error_dialog));
    gtk_widget_destroy(error_dialog);
    return;
  }

  if (num_runners > (end_value - start_value)) {
    std::string str_title = "Invalid number of Runners";
    std::string str_msg = "The number of runners cannot exceed the range\nbetween the start and end value";
    GtkWidget* error_dialog = make_dialog(str_title, str_msg);
    gint result = gtk_dialog_run(GTK_DIALOG(error_dialog));
    gtk_widget_destroy(error_dialog);

    return;
  }


  range_test(start_value, end_value, num_runners, start_max_value, pre_test, geo, label);
}

static void range_test_widget(GtkWidget *wid, GtkWidget *widget) {
  GtkWidget* win = NULL;
  GtkWidget* vbox = NULL;
  GtkWidget* hbox = NULL;

  GtkWidget* from_label = NULL;
  GtkWidget* to_label = NULL;
  GtkWidget* num_runners_label= NULL;
  GtkWidget* status_label = NULL;
  GtkWidget* start_label = NULL;

  GtkWidget* from_entry = NULL;
  GtkWidget* to_entry = NULL;
  GtkWidget* num_runners_entry = NULL;
  GtkWidget* start_entry = NULL;

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

  // The labels #1
  from_label = gtk_label_new("Start of range (>0):");
  gtk_box_pack_start(GTK_BOX(hbox), from_label, TRUE, TRUE, 0);

  to_label = gtk_label_new("End of range:");
  gtk_box_pack_start(GTK_BOX(hbox), to_label, TRUE, TRUE, 0);

  // The entry boxes #1
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

  // The labels #2
  hbox = gtk_hbox_new(TRUE, 0);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);

  num_runners_label = gtk_label_new("The number of Runners");
  gtk_box_pack_start(GTK_BOX(hbox), num_runners_label, TRUE, TRUE, 0);

  start_label = gtk_label_new("The start value of the largest value (minimum is start + num runners)");
  gtk_box_pack_start(GTK_BOX(hbox), start_label, TRUE, TRUE, 0);

  // The entry boxes #2
  hbox = gtk_hbox_new(TRUE, 0);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);

  num_runners_entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(num_runners_entry), "1");
  gtk_box_pack_start(GTK_BOX(hbox), num_runners_entry, TRUE, TRUE, 0);
  range->num_runners = gtk_entry_get_text(GTK_ENTRY(num_runners_entry));

  start_entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(start_entry), "0");
  gtk_box_pack_start(GTK_BOX(hbox), start_entry, TRUE, TRUE, 0);
  range->start_val = gtk_entry_get_text(GTK_ENTRY(start_entry));

   // The check boxes
  hbox = gtk_hbox_new(TRUE, 0);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);
  check_maximum = gtk_check_button_new_with_label("Do pretest");
  gtk_box_pack_start(GTK_BOX(hbox), check_maximum, TRUE, TRUE, 0);
  gtk_widget_set_name(check_maximum, "check_solution");
  range->pre_test = GTK_TOGGLE_BUTTON(check_maximum);

  status_label = gtk_label_new("Status: Not run yet");
  gtk_box_pack_start(GTK_BOX(hbox), status_label, TRUE, TRUE, 0);
  range->label = GTK_LABEL(status_label);

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
		    G_CALLBACK (range_test_entrence),
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

  //  delete options;

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

  geo_time_result* geo_result = NULL;
  num_time_result* num_result = NULL;

  if (geo_check && !(check_for_maximum_solution)) {
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
	std::string str_num_body = str_num_pos;
	// If we have to check, then report the solution here
	if (check_for_maximum_solution) {
	  str_num_body += ", and its maximum distance is\n";
	  RR::SetPrecision(200);
	  RR::SetOutputPrecision(70);

	  RR a = to_RR(num_result->a);
	  RR k = to_RR(num_result->k1) + to_RR(num_result->k2);
	  RR result = a / k;
	  ZZ denominator = to_ZZ(num_result->k1) + to_ZZ(num_result->k2);

	  std::stringstream ss;
	  ss << result << "\n\n";
	  ss << "With Nominator " << num_result-> a << " and denominator " << denominator;
	  str_num_body += ss.str();
	}

	GtkWidget* positive_dialog = make_dialog(str_num_pos, str_num_body);
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

      event_point* point = geo_result->point;
      float f_geo_result = float(point->local_position + point->rounds * (point->number_of_runners + 1)) / float(point->speed * (point->number_of_runners + 1.0));
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


  event_point* point = geo_result->point;
  float f_geo_result = float(point->local_position + point->rounds * (point->number_of_runners + 1)) / float(point->speed * (point->number_of_runners + 1.0));
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

    int candidate =  stringToInt(vec.back().c_str());
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

