#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdio.h>

#include "Geometric.h"
#include "data_structure.h"

static void helloWorld (GtkWidget *wid, GtkWidget *win)
{
  int length = 3;
  int array[length];
  array[0] = 2;
  array[1] = 3;
  array[2] = 5;

   time_result* result = Geometric_method(array, length);
   printf("Time: %f\n", result->result_time);
   free(result);
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

  /* Enter the main loop */
  gtk_widget_show_all (win);
  gtk_main ();
  return 0;
}
