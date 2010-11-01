#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/time.h>
#include <vector>
#include <regex.h>
#include <json/json.h>

#include "Geometric.h"
#include "Numerical.h"
#include "data_structure.h"
#include "util.h"

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
      printf("%s\n", filename);
     
      len_array l_arr = read_json_file_array(filename);
      delete filename;
 
      // test_runner_configuration(l_arr.array, l_arr.len);
      delete l_arr.array;
  }
  gtk_widget_destroy (diag);

}

static void options() {
  
  GtkWidget* win = NULL;
  GtkWidget* vbox = NULL;
  GtkWidget* hbox = NULL;
  GtkWidget* check_widget = NULL;
  GtkWidget* check_maximum = NULL;
  GtkWidget* geo_radio = NULL;
  GtkWidget* num_radio = NULL;
  
  GtkWidget* ok_button = NULL;
  GtkWidget* cancel_button = NULL;

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

  check_widget = gtk_check_button_new_with_label("Find Maximum distance (using Numerical algorithm)");
  gtk_box_pack_start(GTK_BOX(hbox), check_widget, TRUE, TRUE, 0);
  gtk_widget_set_name(check_widget, "check_maximum_distance");

  // The radio buttons
  hbox = gtk_hbox_new(TRUE, 2);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);

  geo_radio = gtk_radio_button_new_with_label(NULL, "Find a solution with the Geometrical algorithm");
  gtk_box_pack_start(GTK_BOX(hbox), geo_radio, TRUE, TRUE, 0);

  num_radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(geo_radio), "Find a solution with the Numerical algorithm");
  gtk_box_pack_start(GTK_BOX(hbox), num_radio, TRUE, TRUE, 0);

  // The buttons
  hbox = gtk_hbox_new(TRUE, 2);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);

  ok_button = gtk_button_new_from_stock ("Run");
  g_signal_connect (G_OBJECT (ok_button), 
		    "clicked", 
		    G_CALLBACK (import), 
		    (gpointer) win);
  gtk_box_pack_start (GTK_BOX (hbox), ok_button, TRUE, TRUE, 0);
  
  cancel_button = gtk_button_new_from_stock ("Cancel");
  g_signal_connect_swapped (G_OBJECT (cancel_button), 
		    "clicked", 
		    G_CALLBACK (gtk_widget_destroy), 
		    G_OBJECT (win) );
  
  gtk_box_pack_start (GTK_BOX (hbox), cancel_button, TRUE, TRUE, 0);

  gtk_widget_show_all (win);
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
      num_time_result* num_result = Numerical_method(array, runnerNum, true, false);
      gettimeofday(end, NULL);
      
      valid = isValid(num_result, array, runnerNum);
      float f_num_result = float(num_result->a) / float(num_result->k1 + num_result->k2);
    
      printf("Time: %f, Valid: %d", f_num_result, valid);
      
      delete point;
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
  num_time_result* num_result = Numerical_method(array, length, true, false);
  gettimeofday(end, NULL);
  
  valid = isValid(num_result, array, length);
  float f_num_result = float(num_result->a) / float(num_result->k1 + num_result->k2);
  
  
  delete geo_result;
  delete num_result;
  delete end;
  delete start;
  
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
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (import), (gpointer) win);
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock ("Import speeds to test");
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (import), (gpointer) win);
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock ("Make range test");
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (import), (gpointer) win);
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock("Options");
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (options), (gpointer) win);
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
