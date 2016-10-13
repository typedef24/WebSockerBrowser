#include <stdio.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <webkit/webkit.h>
#include <unistd.h>

#define MAX_ADDR_LEN 150


/* declaring global variables */
gint num_pages = 0;	//0 page already exist by default
void* retval; 
GtkWidget *notebook;
GtkWidget *window, *webviewer, *scrolled_win;
GtkWidget *newtablabel;
GtkAdjustment *horizontal, *vertical;
GtkWidget *searchentry, *plus, *searchbutton;
const char* uri;
char home_addr[MAX_ADDR_LEN];
	
//declaring fxns
static void file_opener (GtkButton*, GtkNotebook*);
gint delete( GtkWidget* widget, GtkWidget* event, gpointer data );
static void rotate_book( GtkButton* button, GtkNotebook* notebook );
static void go_home( GtkButton* button, GtkNotebook* notebook );
static void tabsborder_book( GtkButton* button, GtkNotebook* notebook );
static void remove_page( GtkButton* button, GtkWidget* notebook );
static void* add_page();
static void addNewThread(GtkButton* button, gpointer data);
static void x_clicked (GtkWidget *parent);
static void helper(GtkWidget* button, GtkWindow* parent);
static void about_us(GtkWidget* button, GtkWidget* parent);
static void forward(GtkWidget* button, gpointer data);
static void back(GtkWidget* button, gpointer data);
static void stop(GtkWidget* button, gpointer data);
void progress (WebKitWebView *webviewer, gint progress, GtkWidget* searchentry);
void finished (WebKitWebView  *webviewer, WebKitWebFrame *frame, 
	       GtkWidget *searchentry);
static void searcher(GtkWidget* button, gpointer data);


int main( int argc, char *argv[] ){
	GtkWidget* togglebutton;	//toggles tab position
	GtkWidget* borderbutton;	//toggle page border on/off
	GtkWidget* plusimage;
	GtkWidget* browsertable;	//represents the whole table
	GtkWidget* menuhbox;		//the topmost menu box
	GtkWidget *help, *hbox, *vbox, *about;
	GtkWidget* filebutton;
	gint i, target = 1;
	PangoFontDescription *initial_font, *initial_font1, *initial_font2;
 
	gtk_init (&argc, &argv);
	
	/* define the browser window */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Web Socker");
	gtk_window_set_default_size (GTK_WINDOW(window), 850, 700);
	gtk_container_set_border_width (GTK_CONTAINER (window), 0);
 	g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (x_clicked), 
                          NULL);
	gtk_window_set_icon_from_file (GTK_WINDOW(window), "LOGO.jpg", NULL);
	while (gtk_events_pending ())
		gtk_main_iteration ();

	//define contents of window: browser table
	browsertable = gtk_table_new (3, 6, FALSE);
	gtk_container_add (GTK_CONTAINER (window), browsertable);

	/* Create the browser notebook and attach it to the browser table */
	notebook = gtk_notebook_new ();
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
	gtk_table_attach(GTK_TABLE (browsertable), notebook, 0, 6, 1, 3,
	                 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 1);
	/*GtkWidget* plusbutton = gtk_button_new_with_label("addtab");
	g_signal_connect (G_OBJECT (plusbutton), "clicked", G_CALLBACK (add_page), 
                          notebook);
	g_signal_emit_by_name (plusbutton, "clicked", notebook);*/
	add_page();

	//load the home page from file
	strcpy(home_addr, "file://");
	strcat(home_addr, g_get_current_dir());
	strcat(home_addr, "/Web_Socker_home.html");
	gtk_entry_set_text(GTK_ENTRY(searchentry), home_addr);
	g_signal_emit_by_name (searchentry, "activate", notebook);

	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
	
	/* Create the menu buttons and attach them to the browser table */
	menuhbox = gtk_hbox_new(FALSE, 5);
	filebutton = gtk_button_new_with_mnemonic ("_File");
	g_object_set(filebutton, "relief", GTK_RELIEF_NONE, NULL);
	g_signal_connect (G_OBJECT (filebutton), "clicked",
                          G_CALLBACK (file_opener),(gpointer) notebook);
	about = gtk_button_new_with_mnemonic("_About");
	g_object_set(about, "relief", GTK_RELIEF_NONE, NULL);
	g_signal_connect (G_OBJECT (about), "clicked", G_CALLBACK (about_us),
			 (gpointer) window);
	help = gtk_button_new_with_mnemonic("_Help");
	g_object_set(help, "relief", GTK_RELIEF_NONE, NULL);
	g_signal_connect (G_OBJECT (help), "clicked", G_CALLBACK (helper),
			  (gpointer) window);
	togglebutton = gtk_button_new_with_mnemonic ("_Tabs position");
	g_object_set(togglebutton, "relief", GTK_RELIEF_NONE, NULL);
	g_signal_connect (G_OBJECT (togglebutton), "clicked",
			  G_CALLBACK (rotate_book),
			  (gpointer) notebook);
	borderbutton = gtk_button_new_with_mnemonic ("_Borders on/off");
	g_object_set(borderbutton, "relief", GTK_RELIEF_NONE, NULL);
	g_signal_connect (G_OBJECT (borderbutton), "clicked",
			  G_CALLBACK (tabsborder_book),
			  (gpointer) notebook);
	gtk_box_pack_start (GTK_BOX (menuhbox), filebutton, FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (menuhbox), togglebutton, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (menuhbox), borderbutton, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (menuhbox), about, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (menuhbox), help, FALSE, FALSE, 0);

	gtk_table_attach(GTK_TABLE(browsertable), menuhbox, 0, 1, 0, 1,
			 GTK_SHRINK, GTK_SHRINK, 1, 0);
	gtk_widget_show_all(window);
	gtk_main ();
return 0;
}

/* This function switches the position of the tabs top/bottom */
void rotate_book( GtkButton* button, GtkNotebook* notebook ){
	if(gtk_notebook_get_tab_pos(notebook) == GTK_POS_TOP)
		gtk_notebook_set_tab_pos(notebook, GTK_POS_BOTTOM);
	else	gtk_notebook_set_tab_pos(notebook, GTK_POS_TOP); 
}

//go_home: sets the home page as the active one
void go_home( GtkButton* button, GtkNotebook* notebook ){
	gtk_notebook_set_current_page(notebook, 0);
	//load the home page from file
	gtk_entry_set_text(GTK_ENTRY(searchentry), home_addr);
	g_signal_emit_by_name (searchentry, "activate", notebook);
}

/* Add/Remove the page tabs and the borders */
void tabsborder_book( GtkButton* button, GtkNotebook* notebook ){
	gint tval = FALSE;
	gint bval = FALSE;
	if (notebook -> show_tabs == 0)
		tval = TRUE;
	if (notebook -> show_border == 0)
 		bval = TRUE;
	gtk_notebook_set_show_tabs (notebook, tval);
	gtk_notebook_set_show_border (notebook, bval);
}

/* Remove a page from the notebook */
void remove_page( GtkButton* button, GtkWidget* notebook ){
 	gint page;
	page = gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook));
	if(page != 0){	//don't remove homepage
		/*gtk_notebook_remove_page (GTK_NOTEBOOK(notebook), page);
		num_pages--;*/
		pthread_exit(&retval);
	}
      /*Need to refresh the widget -- This forces the widget to redraw itself*/
	gtk_widget_queue_draw(notebook);
}

//add_page adds a page to the notebook
static void* add_page(){
	GtkWidget *pagetable;
	GtkWidget *cancel, *cancelimage, *plusimage;
	GtkWidget *enginecombo;
	GtkWidget *searchbox, *searchlabel, *searchimage;
	GtkWidget *homebutton, *vbox, *stoper, *stopimage, *stoplabel, *stopbox;
	gint page_no;
	GtkEntryCompletion *completion;
	GtkListStore *store;
	GtkTreeIter iter;
	GtkWidget *tabtable, *homeimage;
	GtkWidget *rightarrow, *leftarrow, *downarrow;
	GtkWidget *rightarrowimage, *leftarrowimage, *downarrowimage;
	char search[MAX_ADDR_LEN];
	FILE* fp;
	gint i;

	//tabtable holds the tab components
	tabtable = gtk_table_new(1, 3, FALSE);
	if(num_pages == 0)
		newtablabel = gtk_label_new("Web Socker home");
	else newtablabel = gtk_label_new("New Tab");
	plus = gtk_button_new();
	plusimage = gtk_image_new_from_stock(GTK_STOCK_ADD, 1);
	gtk_container_add(GTK_CONTAINER(plus), plusimage);
	g_object_set(plus, "relief", GTK_RELIEF_NONE, NULL);
	cancelimage = gtk_image_new_from_stock(GTK_STOCK_CLOSE, 1);
	cancel = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(cancel), cancelimage);
	g_object_set(cancel, "relief", GTK_RELIEF_NONE, NULL);
	g_signal_connect (G_OBJECT (cancel), "clicked",
			  G_CALLBACK (remove_page),  notebook);
	g_signal_connect (G_OBJECT (plus), "clicked",
			  G_CALLBACK (addNewThread), NULL);
	gtk_table_attach(GTK_TABLE(tabtable), cancel, 0, 1, 0, 1, GTK_SHRINK,
                         GTK_SHRINK, 0, 0);
	gtk_table_attach(GTK_TABLE(tabtable), newtablabel, 1, 2, 0, 1,
			 GTK_EXPAND, GTK_EXPAND, 0, 0);
	gtk_table_attach(GTK_TABLE(tabtable), plus, 2, 3, 0, 1, GTK_SHRINK,
                         GTK_SHRINK, 0, 0);
	gtk_widget_set_size_request(tabtable, 175, 25);
	gtk_widget_show_all(tabtable);
	 
	searchentry = gtk_entry_new();
        gtk_entry_set_text (GTK_ENTRY(searchentry),
			    "http://");
	gtk_widget_grab_focus (searchentry);
	
	//add recent searches into completion from file
	/* if file does not exist */
	if((fp = fopen("recent_Searches.txt", "r")) == NULL){
		//then open in write mode in order 2 create the file and then reopen it
		fp = fopen("recent_Searches.txt", "w");
			system("chmod 744 recent_Searches.txt");
			fopen("recent_Searches.txt", "r");
	}

	/* Create a GtkListStore that will hold autocompletion possibilities. */
	store = gtk_list_store_new (1, G_TYPE_STRING);
	while(!(feof(fp))){
		fgets(search, MAX_ADDR_LEN, fp);
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, 0, search, -1);
	}
	fclose(fp);
	completion = gtk_entry_completion_new ();
	gtk_entry_set_completion (GTK_ENTRY (searchentry), GTK_ENTRY_COMPLETION(completion));
	gtk_entry_completion_set_model (GTK_ENTRY_COMPLETION(completion), GTK_TREE_MODEL (store));
	gtk_entry_completion_set_text_column (GTK_ENTRY_COMPLETION(completion), 0);
	gtk_entry_completion_set_inline_completion(GTK_ENTRY_COMPLETION(completion), TRUE);
	g_object_unref (GTK_ENTRY_COMPLETION(completion));
        g_object_unref (store);

	//create stop  button
	stoper = gtk_button_new();
	stopimage = gtk_image_new_from_stock(GTK_STOCK_STOP, 2);
	stopbox = gtk_hbox_new(FALSE, 0);
	stoplabel = gtk_label_new_with_mnemonic("_Stop");
	gtk_box_pack_start(GTK_BOX(stopbox), stopimage, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(stopbox), stoplabel, FALSE, FALSE, 0);
	gtk_button_set_relief (GTK_BUTTON (stoper), GTK_RELIEF_NONE);
	gtk_container_add(GTK_CONTAINER(stoper), stopbox);
	g_signal_connect (G_OBJECT (stoper), "clicked",
			  G_CALLBACK (stop), NULL);
	
	//create the arrow buttons
	downarrow = gtk_button_new();
	g_object_set(downarrow, "relief", GTK_RELIEF_NONE, NULL);
	downarrowimage = gtk_image_new_from_stock(GTK_STOCK_GO_DOWN, 2);
	gtk_container_add(GTK_CONTAINER(downarrow), downarrowimage);
	leftarrow = gtk_button_new();
	g_object_set(leftarrow, "relief", GTK_RELIEF_NONE, NULL);
	leftarrowimage = gtk_image_new_from_stock(GTK_STOCK_GO_BACK, 2);
	gtk_container_add(GTK_CONTAINER(leftarrow), leftarrowimage);
	g_signal_connect (G_OBJECT (leftarrow), "clicked", G_CALLBACK (back),
			  NULL);
	rightarrow = gtk_button_new();
	g_object_set(rightarrow, "relief", GTK_RELIEF_NONE, NULL);
	rightarrowimage = gtk_image_new_from_stock(GTK_STOCK_GO_FORWARD, 2);
	gtk_container_add(GTK_CONTAINER(rightarrow), rightarrowimage);
	g_signal_connect (G_OBJECT (rightarrow), "clicked", G_CALLBACK (forward),
			  NULL);

	//create the search button
	searchbox = gtk_hbox_new(FALSE, 0);
	searchlabel = gtk_label_new_with_mnemonic("_Search");
	searchimage = gtk_image_new_from_stock(GTK_STOCK_FIND, 2);
	gtk_box_pack_start(GTK_BOX(searchbox), searchimage, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(searchbox), searchlabel, FALSE, FALSE, 0);
        searchbutton = gtk_button_new();
	gtk_button_set_relief (GTK_BUTTON (searchbutton), GTK_RELIEF_NONE);
	gtk_container_add(GTK_CONTAINER(searchbutton), searchbox);
	g_signal_connect (G_OBJECT (searchbutton), "clicked",
			  G_CALLBACK (searcher), notebook);
	g_signal_connect (G_OBJECT (searchentry), "activate",
             		  G_CALLBACK (searcher), notebook);

	//select search engine to use
	enginecombo = gtk_combo_new();
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (enginecombo)->entry),
			    "System Surfer");

	//create the page table
	pagetable = gtk_table_new(1, 14, FALSE);
	//create home button
	homebutton = gtk_button_new();
	g_object_set(homebutton, "relief", GTK_RELIEF_NONE, NULL);
	homeimage = gtk_image_new_from_stock(GTK_STOCK_HOME, 2);
	gtk_container_add(GTK_CONTAINER(homebutton), homeimage);
	g_signal_connect (G_OBJECT (homebutton), "clicked",
			  G_CALLBACK (go_home),
			  (gpointer) notebook);
	
	//create the webview
	webviewer = webkit_web_view_new();
	vbox = gtk_vbox_new(false,false);
	g_signal_connect(WEBKIT_WEB_VIEW(webviewer), "load-progress-changed",
			 G_CALLBACK(progress), searchentry);
	g_signal_connect(WEBKIT_WEB_VIEW(webviewer), "load-finished",
                         G_CALLBACK(finished), searchentry);

	//create a scrolled window & placed the webviewer inside
	scrolled_win = gtk_scrolled_window_new (NULL, NULL);
	horizontal = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (
							  scrolled_win));
	vertical = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (
							scrolled_win));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scrolled_win), webviewer);
	
	/* pack widgets to the page table */
	gtk_table_attach(GTK_TABLE(pagetable), leftarrow, 0, 1, 0, 1, GTK_SHRINK,
                         GTK_SHRINK, 0, 0);
	gtk_table_attach(GTK_TABLE(pagetable), rightarrow, 1, 2, 0, 1, GTK_SHRINK,
                         GTK_SHRINK, 0, 0);
	gtk_table_attach(GTK_TABLE(pagetable), searchentry, 2, 8, 0, 1, 
			 GTK_EXPAND | GTK_FILL, GTK_SHRINK, 1, 0);
	gtk_table_attach(GTK_TABLE(pagetable), stoper, 8, 9, 0, 1, GTK_SHRINK,
                         GTK_SHRINK, 1, 0);
	gtk_table_attach(GTK_TABLE(pagetable), searchbutton, 9, 10, 0, 1, GTK_SHRINK,
                         GTK_SHRINK, 1, 0);
	gtk_table_attach(GTK_TABLE(pagetable), enginecombo, 10, 11, 0, 1,
                         GTK_SHRINK, GTK_SHRINK, 1, 0);
	gtk_table_attach(GTK_TABLE(pagetable), downarrow, 11, 12, 0, 1, GTK_SHRINK,
                         GTK_SHRINK, 1, 0);
	gtk_table_attach(GTK_TABLE(pagetable), homebutton, 12, 13, 0, 1, GTK_SHRINK,
                         GTK_SHRINK, 0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), pagetable, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_win, TRUE, TRUE, 1);

	gtk_widget_show_all(pagetable);

	/* insert a page to the notebook */
	if(num_pages == 0){	//if no page already exist
	     gtk_notebook_insert_page (GTK_NOTEBOOK (notebook), vbox, tabtable, 0);
             gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0); 
	}
	else{
		page_no = gtk_notebook_get_current_page(GTK_NOTEBOOK (notebook));
		gtk_notebook_insert_page (GTK_NOTEBOOK (notebook), vbox, tabtable,
					  page_no + 1);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), page_no + 1); 
	}
	num_pages++;
	gtk_widget_show_all(notebook);
}

//notify user when closing multiple tabs
static void x_clicked (GtkWidget *parent){
	GtkWidget *dialog;
	gint result;
	dialog = gtk_message_dialog_new (GTK_WINDOW(parent), GTK_DIALOG_MODAL,
					GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
					"You are about to close multiple tabs\nYou might lose data in the process!\nDo you still want to continue?");
	gtk_window_set_title (GTK_WINDOW (dialog), "CONFIRM CLOSE");
	if(num_pages > 1 ){
		result = gtk_dialog_run (GTK_DIALOG (dialog));
		if(result == GTK_RESPONSE_NO)
			gtk_widget_destroy (dialog);
		else{
			gtk_widget_destroy (parent);	
			gtk_widget_destroy (dialog);
			gtk_main_quit();
		}
	}
	else{ 
		gtk_widget_destroy(parent);
		gtk_main_quit();
	}
}

//select a file and open with the browser
static void file_opener (GtkButton *button, GtkNotebook *notebook){
	GtkWidget *dialog;
	gchar *filename;
	char path[100];
	strcpy(path, "file://");
      dialog = gtk_file_chooser_dialog_new ("Open an ofline web page with Web Socker", 						       GTK_WINDOW(window),
						GTK_FILE_CHOOSER_ACTION_OPEN,
						GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
						NULL);
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));
	if (result == GTK_RESPONSE_ACCEPT){
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		//load the selected page from file
		strcat(path, filename);
		gtk_entry_set_text(GTK_ENTRY(searchentry), path);
		g_signal_emit_by_name (searchentry, "activate", notebook);

	}
	gtk_widget_destroy (dialog);
}

static void helper(GtkWidget* button, GtkWindow* parent){
	GtkWidget *dialog, *label, *image, *hbox;
	/* Create a new dialog with one OK button. */
       dialog = gtk_dialog_new_with_buttons ("Web Socker Help", NULL, GTK_DIALOG_MODAL,
                                       	     GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	label = gtk_label_new ("Web Socker is very easy to use!\n-> To browse online, prefix your search string or address with http://\n-> To interprete ofline web pages like html, css, php, javascript, etc:\n   -Prefix your local address with file:// followed by the absolute path to the web document.\n   -You can simply do this by clicking File in the top menu and selecting the web document from your file system.");
	image = gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_DIALOG);
	hbox = gtk_hbox_new (FALSE, 5);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
	gtk_box_pack_start_defaults (GTK_BOX (hbox), image);
	gtk_box_pack_start_defaults (GTK_BOX (hbox), label);
	/* Pack the dialog content into the dialog's GtkVBox. */
	gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox);
	gtk_widget_show_all (dialog);
	/* Create the dialog as modal and destroy it when a button is clicked. */
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

static void about_us(GtkWidget* button, GtkWidget* parent){
	GtkWidget *dialog;
	GdkPixbuf *logo;
	GError *error = NULL;
	const gchar *authors[] = {"-> MOFORKENG STEPHEN NEMBO (leader)",
				  "-> BAAR DAVID C.",
				  "-> PEMBE MIRIAM",
				  "-> APOUNDA SADO GILLES",
				  "-> NKWELLE STEPHEN",
				  "-> NGOUATOU AURELIEN",
				  "-> POKEM JULIO CESAR",
				  "Special thanks to Mr Ngatchu Damen for his inspiration", NULL};
	const gchar *documenters[] = {"-> MOFORKENG STEPHEN NEMBO (leader)",
				  "-> BAAR DAVID C.",
				  "-> PEMBE MIRIAM",
				  "-> APOUNDA SADO GILLES",
				  "-> NKWELLE STEPHEN",
				  "-> NGOUATOU AURELIEN",
				  "-> POKEM JULIO CESAR", NULL};

	dialog = gtk_about_dialog_new ();
	logo = gdk_pixbuf_new_from_file ("LOGO.jpg", &error);
	gdk_pixbuf_scale_simple(logo, 2, 2, GDK_INTERP_BILINEAR);
	
	/* Set the application logo or handle the error. */
	if (error == NULL)
		gtk_about_dialog_set_logo (GTK_ABOUT_DIALOG (dialog), logo);
	else{
		if (error->domain == GDK_PIXBUF_ERROR)
			g_print ("GdkPixbufError: %s\n", error->message);
		else if (error->domain == G_FILE_ERROR)
			g_print ("GFileError: %s\n", error->message);
		else
			g_print ("An error in the domain: %d has occurred!\n",
				  error->domain);
		g_error_free (error);
	}
	/* Set application data that will be displayed in the main dialog. */
	gtk_about_dialog_set_name (GTK_ABOUT_DIALOG (dialog),
				   "Web Socker Browser");
	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (dialog), "1.0");
	gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (dialog),
					"(C) 2016 Web Socker Browser");
	gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG (dialog),
	 	"All About Globalization\nSearching From Within And Without Using Only Web Socker Browser");
	/* Set the license text, which is usually loaded from a file. Also, set the
	* web site address and label. */
	gtk_about_dialog_set_license (GTK_ABOUT_DIALOG (dialog), "This is a free open source product");
	gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (dialog),
				      "http://webSocker.com");
	gtk_about_dialog_set_website_label (GTK_ABOUT_DIALOG (dialog),
					    "webSoker.com");
	/* Set the application authors, documenters and translators. */
	gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (dialog), authors);
	gtk_about_dialog_set_documenters (GTK_ABOUT_DIALOG (dialog), documenters);
	gtk_about_dialog_set_translator_credits (GTK_ABOUT_DIALOG (dialog),
						 "Web Socker is currently just in one language, the English language.");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

//searcher() loads a given uri from the gtk entry.
void searcher(GtkWidget* button, gpointer data){
	char search[MAX_ADDR_LEN];
	int counter = 0;
	FILE* fp;
	uri = gtk_entry_get_text(GTK_ENTRY(searchentry));
	gtk_entry_progress_pulse(GTK_ENTRY(searchentry));
	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webviewer), uri);
	//add uri to recent_Searches.txt
	if((fp = fopen("recent_Searches.txt", "a+")) == NULL){
		printf("Error openx recent_Searches.txt in finished()!");
		exit(0);
	}
	while(!(feof(fp))){
		fscanf(fp, "%s", search);
		if(strcmp(search, gtk_entry_get_text(GTK_ENTRY(searchentry))) == 0)				counter++;
	}
	if(counter == 0){
		fprintf(fp, "%s\n", gtk_entry_get_text(GTK_ENTRY(searchentry)));
		fflush(fp);
	}
fclose(fp);
}

//go one page backward
void back(GtkWidget* button, gpointer data){
	webkit_web_view_go_back(WEBKIT_WEB_VIEW(webviewer));
}

//go one web page forewrd
void forward(GtkWidget* button, gpointer data){
	webkit_web_view_go_forward(WEBKIT_WEB_VIEW(webviewer));
}

//stop loading a page
void stop(GtkWidget* button, gpointer data){
	webkit_web_view_stop_loading(WEBKIT_WEB_VIEW(webviewer));
}

//check loading progress
void progress (WebKitWebView *webviewer, gint progress, GtkWidget* searchentry){
	gtk_entry_set_progress_fraction (GTK_ENTRY(searchentry),progress);
}
 
//check if loading is finished
void finished (WebKitWebView  *webviewer, WebKitWebFrame *frame, 
	       GtkWidget *searchentry){
	char label[16];
	gtk_entry_set_progress_fraction (GTK_ENTRY(searchentry), 0.0);
}

//addNewThread: creates a new thread to run a tab.
static void addNewThread(GtkButton* button, gpointer data){
	pthread_t threadId;
	int status;
       if((status = pthread_create(&threadId, NULL, add_page,
	 NULL)) != 0){
		perror("Error! pthread_create(): ");	
	}
}

