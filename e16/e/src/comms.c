/*
 * Copyright (C) 2000-2004 Carsten Haitzler, Geoff Harrison and various contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "E.h"

struct _client
{
   char               *name;
   Window              win;
   char               *msg;
   char               *clientname;
   char               *version;
   char               *info;
   char                replied;
};

static Window       comms_win = 0;

static Atom         XA_ENLIGHTENMENT_COMMS = 0;
static Atom         XA_ENL_MSG = 0;

static void         ClientHandleEvents(XEvent * ev, void *cc);

static Client      *
ClientCreate(Window win)
{
   Client             *c;
   char                st[32];

   c = Ecalloc(1, sizeof(Client));
   if (!c)
      return NULL;

   Esnprintf(st, sizeof(st), "%8x", (int)win);
   c->name = Estrdup(st);
   c->win = win;
   ERegisterWindow(win);
   EventCallbackRegister(win, 0, ClientHandleEvents, c);
   AddItem(c, st, win, LIST_TYPE_CLIENT);
   XSelectInput(disp, win, StructureNotifyMask | SubstructureNotifyMask);

   return c;
}

static void
ClientDestroy(Client * c)
{
   Window              win;

   if (!c)
      return;

   win = c->win;
   RemoveItem(NULL, win, LIST_FINDBY_ID, LIST_TYPE_CLIENT);
   EventCallbackUnregister(win, 0, ClientHandleEvents, c);
   EUnregisterWindow(win);
   if (c->name)
      Efree(c->name);
   if (c->msg)
      Efree(c->msg);
   if (c->clientname)
      Efree(c->clientname);
   if (c->version)
      Efree(c->version);
   if (c->info)
      Efree(c->info);
   Efree(c);
}

static int
ClientConfigure(Client * c, const char *str)
{
   char                param[64];
   const char         *value;
   int                 len;

   len = 0;
   sscanf(str, "%*s %60s %n", param, &len);
   value = str + len;

   if (!strcmp(param, "clientname"))
     {
	if (c->clientname)
	   Efree(c->clientname);
	c->clientname = Estrdup(value);
     }
   else if (!strcmp(param, "version"))
     {
	if (c->version)
	   Efree(c->version);
	c->version = Estrdup(value);
     }
   else if (!strcmp(param, "author"))
     {
     }
   else if (!strcmp(param, "email"))
     {
     }
   else if (!strcmp(param, "web"))
     {
     }
   else if (!strcmp(param, "address"))
     {
     }
   else if (!strcmp(param, "info"))
     {
	if (c->info)
	   Efree(c->info);
	c->info = Estrdup(value);
     }
   else if (!strcmp(param, "pixmap"))
     {
     }
   else
     {
	return -1;
     }

   return 0;
}

static char        *
ClientCommsGet(Client ** c, XClientMessageEvent * ev)
{
   char                s[13], s2[9], *msg;
   int                 i;
   Window              win = 0;
   Client             *cl;

   if ((!ev) || (!c))
      return NULL;
   if (ev->message_type != XA_ENL_MSG)
      return NULL;

   s[12] = 0;
   s2[8] = 0;
   msg = NULL;
   for (i = 0; i < 8; i++)
      s2[i] = ev->data.b[i];
   for (i = 0; i < 12; i++)
      s[i] = ev->data.b[i + 8];
   sscanf(s2, "%lx", &win);
   cl = (Client *) FindItem(NULL, win, LIST_FINDBY_ID, LIST_TYPE_CLIENT);
   if (!cl)
     {
	cl = ClientCreate(win);
	if (!cl)
	   return NULL;
     }

   if (cl->msg)
     {
	/* append text to end of msg */
	cl->msg = Erealloc(cl->msg, strlen(cl->msg) + strlen(s) + 1);
	if (!cl->msg)
	   return NULL;
	strcat(cl->msg, s);
     }
   else
     {
	/* new msg */
	cl->msg = Emalloc(strlen(s) + 1);
	if (!cl->msg)
	   return NULL;
	strcpy(cl->msg, s);
     }
   if (strlen(s) < 12)
     {
	msg = cl->msg;
	cl->msg = NULL;
	*c = cl;
     }
   return msg;
}

static void
ClientHandleComms(XClientMessageEvent * ev)
{
   Client             *c;
   char               *s;
   const char         *s1, *s2;

   s = ClientCommsGet(&c, ev);
   if (!s)
      return;

   if (EventDebug(EDBUG_TYPE_IPC))
      Eprintf("ClientHandleComms: %s\n", s);

   if (!strncmp(s, "set ", 4))
     {
	/* The old Client set command (used by epplets) */
	if (ClientConfigure(c, s) == 0)
	   goto done;
     }

   if (!HandleIPC(s, c))
     {
	s1 = (c->clientname) ? c->clientname : "UNKNOWN";
	s2 = (c->version) ? c->version : "UNKNOWN";
	DialogOK(_("E IPC Error"),
		 _("Received Unknown Client Message.\n"
		   "Client Name:    %s\n" "Client Version: %s\n"
		   "Message Contents:\n\n" "%s\n"), s1, s2, s);
	SoundPlay("SOUND_ERROR_IPC");
     }

 done:
   Efree(s);
}

static void
ClientHandleEvents(XEvent * ev, void *cc)
{
   Client             *c = (Client *) cc;

#if 0
   Eprintf("ClientHandleEvents: type=%d win=%#lx\n", ev->type, ev->xany.window);
#endif
   switch (ev->type)
     {
     case DestroyNotify:
	ClientDestroy(c);
	break;
     case ClientMessage:
	ClientHandleComms(&(ev->xclient));
	break;
     }
}

void
CommsInit(void)
{
   char                s[1024];

   comms_win = XCreateSimpleWindow(disp, VRoot.win, -100, -100, 5, 5, 0, 0, 0);
   ERegisterWindow(comms_win);
   XSelectInput(disp, comms_win, StructureNotifyMask | SubstructureNotifyMask);
   EventCallbackRegister(comms_win, 0, ClientHandleEvents, NULL);

   Esnprintf(s, sizeof(s), "WINID %8x", (int)comms_win);
   XA_ENLIGHTENMENT_COMMS = XInternAtom(disp, "ENLIGHTENMENT_COMMS", False);
   XChangeProperty(disp, comms_win, XA_ENLIGHTENMENT_COMMS, XA_STRING, 8,
		   PropModeReplace, (unsigned char *)s, strlen(s));
   XChangeProperty(disp, VRoot.win, XA_ENLIGHTENMENT_COMMS, XA_STRING, 8,
		   PropModeReplace, (unsigned char *)s, strlen(s));

   XA_ENL_MSG = XInternAtom(disp, "ENL_MSG", False);
}

static void
CommsDoSend(Window win, const char *s)
{
   char                ss[21];
   int                 i, j, k, len;
   XEvent              ev;

   if ((!win) || (!s))
      return;

   len = strlen(s);
   ev.xclient.type = ClientMessage;
   ev.xclient.serial = 0;
   ev.xclient.send_event = True;
   ev.xclient.window = win;
   ev.xclient.message_type = XA_ENL_MSG;
   ev.xclient.format = 8;
   for (i = 0; i < len + 1; i += 12)
     {
	Esnprintf(ss, sizeof(ss), "%8x", (int)comms_win);
	for (j = 0; j < 12; j++)
	  {
	     ss[8 + j] = s[i + j];
	     if (!s[i + j])
		j = 12;
	  }
	ss[20] = 0;
	for (k = 0; k < 20; k++)
	   ev.xclient.data.b[k] = ss[k];
	XSendEvent(disp, win, False, 0, (XEvent *) & ev);
     }
}

void
CommsSend(Client * c, const char *s)
{
   if (!c)
      return;

   c->replied = 1;
   CommsDoSend(c->win, s);
}

void
CommsFlush(Client * c)
{
   if (!c)
      return;

   if (!c->replied)
      CommsDoSend(c->win, "");
}

/*
 * When we are running in multi-head, connect to the master wm process
 * and send the message
 */
void
CommsSendToMasterWM(const char *s)
{
   if (Mode.wm.master)
      return;

   CommsDoSend(RootWindow(disp, Mode.wm.master_screen), s);
}

/*
 * When we are running in multi-head, connect to the slave wm processes
 * and broadcast the message
 */
void
CommsBroadcastToSlaveWMs(const char *s)
{
   int                 screen;

   if (!Mode.wm.master || Mode.wm.single)
      return;

   for (screen = 0; screen < Mode.display.screens; screen++)
     {
	if (screen != Mode.wm.master_screen)
	   CommsDoSend(RootWindow(disp, screen), s);
     }
}

void
CommsBroadcast(const char *s)
{
   char              **l;
   int                 num, i;
   Client             *c;

   l = ListItems(&num, LIST_TYPE_CLIENT);
   if (!s)
      return;
   for (i = 0; i < num; i++)
     {
	c = (Client *) FindItem(l[i], 0, LIST_FINDBY_NAME, LIST_TYPE_CLIENT);
	if (c)
	   CommsSend(c, s);
     }
   freestrlist(l, num);
}
