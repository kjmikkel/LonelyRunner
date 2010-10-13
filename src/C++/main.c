#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <sys/time.h>
#include <vector>

#include "Geometric.h"
#include "Numerical.h"
#include "data_structure.h"

static void helloWorld (GtkWidget *wid, GtkWidget *win) {

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
      time_result* result = Geometric_method(array, runnerNum);
      gettimeofday(end, NULL);
      
      bool valid = isValid(result, array, runnerNum); 
      
      printf("Time: %f, Valid: %i, Time: %d\n", result->result_time, valid, (float)(end->tv_usec - start->tv_usec));
            
      gettimeofday(start, NULL);
      result = Numerical_method(array, runnerNum);
      gettimeofday(end, NULL);
      
      valid = isValid(result, array, runnerNum);
          
      printf("Time: %f, Valid: %d, Time: %d\n", result->result_time, valid, (float)(end->tv_usec - start->tv_usec));
      
      delete result;
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
  time_result* result = Geometric_method(array, length);
  gettimeofday(end, NULL);
  
  bool valid = true;
  bool print = true;
  float compareTo = 1.0 / (length + 1.0);
  for(int index = 0; index < length; index++) {
    valid &= closeToInteger(result->result_time, array[index]) >= compareTo;
  }
  
  bool solution = checkForSolution(array, length);

  printf("Can we be sure there is a solution?: %i\n", solution);
  
  
  
  printf("Time: %f, Valid: %i, Time: %f \n", result->result_time, valid, (float)(end->tv_usec - start->tv_usec));
  
  gettimeofday(start, NULL);
  result = Numerical_method(array, length);
  gettimeofday(end, NULL);
  
  valid = true;
  for(int index = 0; index < length; index++) {
    valid &= closeToInteger(result->result_time, array[index]) >= compareTo;
  }

  printf("val1: %f, val: %f\n", (float)start->tv_usec, (float)end->tv_usec);

  printf("Time: %f, Valid: %d, Time: %f \n", result->result_time, valid, (float)(end->tv_usec - start->tv_usec));
  
  delete result;
  delete end;
  delete start;
}

int main (int argc, char *argv[])
{
  GtkWidget *button = NULL;
  GtkWidget *win = NULL;
  GtkWidget *vbox = NULL;

  /* Initialize GTK+ */
  g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
  gtk_init (&argc, &argv);
  g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

  /* Create the main window */
  win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (win), 8);
  gtk_window_set_title (GTK_WINDOW (win), "Hello World");
  gtk_window_set_position (GTK_WINDOW (win), GTK_WIN_POS_CENTER);
  gtk_widget_realize (win);
  g_signal_connect (win, "destroy", gtk_main_quit, NULL);

  /* Create a vertical box with buttons */
  vbox = gtk_vbox_new (TRUE, 6);
  gtk_container_add (GTK_CONTAINER (win), vbox);

  button = gtk_button_new_from_stock (GTK_STOCK_DIALOG_INFO);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (helloWorld), (gpointer) win);
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  g_signal_connect (button, "clicked", gtk_main_quit, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

  testCustom();

  /* Enter the main loop */
  //gtk_widget_show_all (win);
  //gtk_main ();
  return 0;
}
