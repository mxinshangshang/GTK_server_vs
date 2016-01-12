/*************************************************
* server.c                                    		  *
*   create the server window    		  *
*   send message to client          		  *
*************************************************/
#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define _WIN32_ 1

#ifdef _WIN32_
#include <windows.h>
#endif

#ifdef _LINUX_
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <semaphore.h>
#endif
#define bool _Bool
#define true 1
#define false 0
#define MAXSIZE 1024
//bool socket_start;
GtkTextBuffer *show_buffer, *input_buffer;

gint ad1 = 0;
gint p1 = 0;
gchar buffer[45];

void quit_win(GtkWidget *, gpointer);

gboolean from_host;//表示客户端是否已经向本服务器发送来信息
GSocket *sock;
GSocket *cl_sock;
int issucceed = -1;
int cl_sockfd;
struct sockaddr_in saddr, caddr;//saddr表示本地服务器地址信息，caddr用于保存向本服务器发送信息的主机地址信息

gchar *_(gchar *c)
{
	return(g_locale_to_utf8(c, -1, NULL, NULL, NULL));
}

/* show errors such as "no input","haven't create sockt" etc. */
void show_err(char *err)
{
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(show_buffer), &start, &end);/*获得缓冲区开始和结束位置的Iter*/
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(show_buffer), &end, err, strlen(err));
}

/* show the received message */
void show_remote_text(char rcvd_mess[])
{
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(show_buffer), &start, &end);/*获得缓冲区开始和结束位置的Iter*/
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(show_buffer), &end, "Client:\n", 8);/*插入文本到缓冲区*/
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(show_buffer), &end, rcvd_mess, strlen(rcvd_mess));/*插入文本到缓冲区*/
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(show_buffer), &end, "\n", 1);/*插入换行到缓冲区*/
}

/* Receive function */
gpointer recv_func(gpointer arg)/*recv_func(void *arg)*/
{
	char rcvd_mess[MAXSIZE];
	GInputVector vector;
	GError *error = NULL;
	vector.buffer = rcvd_mess;
	vector.size = MAXSIZE;
	while (1)
	{
		//memset(rcvd_mess, 0, MAXSIZE);
		//if (g_socket_receive(cl_sock, vector.buffer, vector.size, NULL, &error)<0)
		//{
		//	perror("server recv error\n");
		//	exit(1);
		//}
		//g_print("Messages = %s\n", rcvd_mess);
		//show_remote_text(rcvd_mess);
		//g_print("Waiting……");
	}
}

gint build_socket(void *arg)
{
	//g_type_init();
	GInetAddress *iface_address = g_inet_address_new_from_string("127.0.0.1"); //("111.186.100.210");
	GSocketAddress *connect_address = g_inet_socket_address_new(iface_address, 8500);
	GError *err = NULL;
	sock = g_socket_new(G_SOCKET_FAMILY_IPV4,
		G_SOCKET_TYPE_STREAM,
		G_SOCKET_PROTOCOL_TCP,
		&err);
	g_assert(err == NULL);

	if (g_socket_bind(sock,
		connect_address,
		TRUE,
		&err) != TRUE)
	{
		show_err("Bind Error");
		exit(1);
	}
	if (g_socket_listen(sock, &err) != TRUE)
	{
		show_err("Listen Error");
		exit(1);
	}
	while (1)
	{
		cl_sock = g_socket_accept(sock, NULL, &err);
		/* Create a sub thread,call recv_func() */
		g_thread_new("recv_func", recv_func, cl_sock);
		g_print("recv_func start...\n");
		return 0;
		from_host = true;
	}
}

/* Send function,send message to client */
void send_func(const char *text)
{
	int n;
	GError *err = NULL;
	n = g_socket_send(cl_sock,
		text,
		45,
		NULL,
		&err);
	if (n<0)
	{
		perror("S send error\n");
		exit(1);
	}
}

/* show the input text */
void show_local_text(const gchar* text)
{
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(show_buffer), &start, &end);/*获得缓冲区开始和结束位置的Iter*/
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(show_buffer), &end, "Me:\n", 4);/*插入文本到缓冲区*/
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(show_buffer), &end, text, strlen(text));/*插入文本到缓冲区*/
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(show_buffer), &end, "\n", 1);/*插入文本到缓冲区*/
}
/* clean the input text */
void clean_send_text()
{
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(input_buffer), &start, &end);/*获得缓冲区开始和结束位置的Iter*/
	gtk_text_buffer_delete(GTK_TEXT_BUFFER(input_buffer), &start, &end);/*插入到缓冲区*/
}
/* get the input text,and send it */
void send_text()
{
	gint i;
	gdouble P1, AD1;
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(input_buffer), &start, &end);
	//gchar buffer[] = { 0x55, 0x55, 0x55, 0x55,
	//	0x08, 0x08, 0x08, 0x01,
	//	0x02, 0x02, 0x02, 0x02,
	//	0x02, 0x02, 0x02, 0x02,
	//	0x02, 0x02, 0x02, 0x02,
	//	0x03, 0x03, 0x03, 0x03,
	//	0x03, 0x03, 0x03, 0x03,
	//	0x03, 0x03, 0x03, 0x03,
	//	0x03, 0x03, 0x03, 0x03, 0x04,
	//	0x05, 0x05, 0x05, 0x05,
	//	0xaa, 0xaa, 0xaa, 0xaa };

	//for (i = 0; i < 45; i++)
	//{
	//	buffer[i] = 0x00;
	//}
	//buffer[0] = 0x55;
	//buffer[1] = 0x55;
	//buffer[2] = 0x55;
	//buffer[3] = 0x55;
	//buffer[41] = 0xaa;
	//buffer[42] = 0xaa;
	//buffer[43] = 0xaa;
	//buffer[44] = 0xaa;
	//buffer[9] = p1 >> 16;
	//buffer[10] = (p1 & 0xffff) >> 8;
	//buffer[11] = p1 & 0xff;
	//buffer[21] = ad1 >> 16;
	//buffer[22] = (ad1 & 0xffff) >> 8;
	//buffer[23] = ad1 & 0xff;
	//send_func(buffer);
	//P1 = ((gdouble)((buffer[9] & 0x7f) << 16 | buffer[10] << 8 | buffer[11]) / (gdouble)0x7fffff) * 600;
	//AD1 = ((gdouble)((buffer[21] & 0x7f) << 16 | buffer[22] << 8 | buffer[23]) / (gdouble)0x7fffff) * 600;
	//g_print("%.4lf   %.4lf\n", P1, AD1);
	//if (P1>8) 
	//{
	//	p1++;
	//	ad1 = log10(p1);
	//}
	//else
	//{
	//	p1++;
	//	ad1 = p1*50;
	//}
	gdouble diff = 0.1;
	while (1)
	{
		for (i = 0; i < 45; i++)
		{
			buffer[i] = 0x00;
		}
		buffer[0] = 0x55;
		buffer[1] = 0x55;
		buffer[2] = 0x55;
		buffer[3] = 0x55;
		buffer[41] = 0xaa;
		buffer[42] = 0xaa;
		buffer[43] = 0xaa;
		buffer[44] = 0xaa;
		buffer[9] = p1 >> 16;
		buffer[10] = (p1 & 0xffff) >> 8;
		buffer[11] = p1 & 0xff;
		buffer[21] = ad1 >> 16;
		buffer[22] = (ad1 & 0xffff) >> 8;
		buffer[23] = ad1 & 0xff;
		send_func(buffer);
		P1 = ((gdouble)((buffer[9] & 0x7f) << 16 | buffer[10] << 8 | buffer[11]) / (gdouble)0x7fffff) * 600;
		AD1 = ((gdouble)((buffer[21] & 0x7f) << 16 | buffer[22] << 8 | buffer[23]) / (gdouble)0x7fffff) * 600;
		//g_print("%.4lf   %.4lf\n", P1, AD1);
		//if (p1>(0x7fffff / 150))
		//{
		//	p1 = p1 + 80;
		//	//ad1 = (sqrt(3600 * (gdouble)p1 * 600 / 0x7fffff) + 30) * 0x7fffff / 600;
		//	//ad1 = (log10(p1 * 600 / 0x7fffff) + 149.398) * 0x7fffff / 600;
		//}
		//else
		//{
		//	p1 = p1 + 80;
		//	ad1 = p1 * 37.5;
		//}
		if (ad1>(0x7fffff / 4))
		{
			ad1 = (gdouble)ad1 + 800;
			p1 = (0.001066666*((gdouble)ad1 * 600 / 0x7fffff)*((gdouble)ad1 * 600 / 0x7fffff) - 0.2933333*((gdouble)ad1 * 600 / 0x7fffff) + 24) * 0x7fffff / 600;
		}
		else
		{
			ad1 = ad1 + 800;
			p1 = (gdouble)ad1 / 37.5;
		}

		Sleep(10);
	}
}
void startup(void)
{
	//gint i;
	//gdouble P1, AD1;
	//for (i = 0; i < 45; i++)
	//{
	//	buffer[i] = 0x00;
	//}
	//buffer[0] = 0x55;
	//buffer[1] = 0x55;
	//buffer[2] = 0x55;
	//buffer[3] = 0x55;
	//buffer[41] = 0xaa;
	//buffer[42] = 0xaa;
	//buffer[43] = 0xaa;
	//buffer[44] = 0xaa;
	//buffer[9] = p1 >> 16;
	//buffer[10] = (p1 & 0xffff) >> 8;
	//buffer[11] = p1 & 0xff;
	//buffer[21] = ad1 >> 16;
	//buffer[22] = (ad1 & 0xffff) >> 8;
	//buffer[23] = ad1 & 0xff;
	////send_func(buffer);
	//P1 = ((gdouble)((buffer[9] & 0x7f) << 16 | buffer[10] << 8 | buffer[11]) / (gdouble)0x7fffff) * 600;
	//AD1 = ((gdouble)((buffer[21] & 0x7f) << 16 | buffer[22] << 8 | buffer[23]) / (gdouble)0x7fffff) * 600;
	//g_print("%.4lf   %.4lf\n", P1, AD1);
	//ad1++;
	//p1++;


	g_thread_new("build_socket", (GThreadFunc)build_socket, NULL);
	show_err("The server has been started !\n");
	return;
}

/* quit */
void quit_win(GtkWidget *window, gpointer data)
{
	gtk_main_quit();
}

int main(int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *show_text, *input_text;
	GtkWidget *send_button, *quit_button, *cls_button, *start_button;
	GtkWidget *hbox, *vbox;
	GtkWidget *scrolled1, *scrolled2;
	GtkWidget *space_label;
	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Server");
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 430, 320);
	/* "quit" button */
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(quit_win), NULL);

	space_label = gtk_label_new("                          ");
	/* set button */
	send_button = gtk_button_new_with_label("Send");
	quit_button = gtk_button_new_with_label("Close");
	cls_button = gtk_button_new_with_label("Clear");
	start_button = gtk_button_new_with_label("Startup");
	/* set textbox */
	show_text = gtk_text_view_new();
	input_text = gtk_text_view_new();
	/* get the buffer of textbox */
	show_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(show_text));
	input_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(input_text));
	/* set textbox to diseditable */
	gtk_text_view_set_editable(GTK_TEXT_VIEW(show_text), FALSE);
	/* scroll window */
	scrolled1 = gtk_scrolled_window_new(NULL, NULL);
	scrolled2 = gtk_scrolled_window_new(NULL, NULL);
	/* create a textbox */
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled1), show_text);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled2), input_text);
	/* setting of window */
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled2), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);//gtk_hbox_new(FALSE,2);
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);//gtk_vbox_new(FALSE,2);
													/* click quit to call quit_win*/
	g_signal_connect(G_OBJECT(quit_button), "clicked", G_CALLBACK(quit_win), NULL);
	/* click to clear input screen*/
	g_signal_connect(G_OBJECT(cls_button), "clicked", G_CALLBACK(clean_send_text), NULL);
	/* click Start to build socket*/
	g_signal_connect(G_OBJECT(start_button), "clicked", G_CALLBACK(startup), NULL);
	/* create window */
	gtk_box_pack_start(GTK_BOX(hbox), start_button, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), space_label, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), send_button, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), cls_button, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), quit_button, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled1, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled2, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	/* click send button ,then call send_text*/
	g_signal_connect(G_OBJECT(send_button), "clicked", G_CALLBACK(send_text), NULL);
	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}
