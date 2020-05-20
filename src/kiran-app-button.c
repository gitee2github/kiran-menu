#include "kiran-app-button.h"


enum {
    SIGNAL_APP_LAUNCHED=0,
    SIGNAL_MAX
};

struct _KiranAppButton {
    GtkButton parent;

    GtkWidget *icon;
    gchar **exec_args;
};

G_DEFINE_TYPE(KiranAppButton, kiran_app_button, GTK_TYPE_BUTTON)


static guint signals[SIGNAL_MAX];

void kiran_app_button_init(KiranAppButton *self)
{
    GtkStyleContext *context;

    context = gtk_widget_get_style_context(GTK_WIDGET(self));
    self->icon = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(self), self->icon);

    gtk_widget_set_hexpand(GTK_WIDGET(self), TRUE);
    gtk_widget_set_halign(GTK_WIDGET(self), GTK_ALIGN_FILL);

    gtk_style_context_add_class(context, "kiran-app-button");
}

void kiran_app_button_finalize(GObject *obj)
{
    KiranAppButton *button = KIRAN_APP_BUTTON(obj);

    g_free(button->exec_args);
    G_OBJECT_CLASS(kiran_app_button_parent_class)->finalize(obj);
}

void kiran_app_button_clicked(GtkButton *button)
{
    GError *error = NULL;
    GPid pid;
    char *command;
    KiranAppButton *app_btn = KIRAN_APP_BUTTON(button);

    //GTK_BUTTON_CLASS(kiran_app_button_parent_class)->clicked(button);

    command = g_strjoinv(" ", app_btn->exec_args);
#if 1
    if (!g_spawn_async(NULL, app_btn->exec_args, NULL,
            G_SPAWN_SEARCH_PATH, NULL, NULL, &pid, &error)) {

        g_error("Failed to run command '%s': %s\n", command, error->message);
        g_error_free(error);
    } else
    {
        g_signal_emit(app_btn, signals[SIGNAL_APP_LAUNCHED], 0);
        g_debug("child %d spawned\n", pid);
    }
#endif
    g_message("run command '%s'\n", command);
    g_free(command);
    //
}

void kiran_app_button_class_init(KiranAppButtonClass *kclass)
{
    signals[SIGNAL_APP_LAUNCHED] = g_signal_new("app-launched", G_TYPE_FROM_CLASS(kclass),
            G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
    GTK_BUTTON_CLASS(kclass)->clicked = kiran_app_button_clicked;
}


KiranAppButton *kiran_app_button_new(const gchar *icon_file, const gchar *tooltip, const gchar *exec)
{
    KiranAppButton *button;

    if (!exec || !icon_file || !tooltip)
        return NULL;

    button = g_object_new(KIRAN_TYPE_APP_BUTTON, NULL);
    gtk_image_set_from_resource(GTK_IMAGE(button->icon), icon_file);
    gtk_widget_set_tooltip_text(GTK_WIDGET(button), tooltip);

    button->exec_args = g_strsplit(exec, " ", -1);

    return button;
}
