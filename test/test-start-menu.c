#include <gio/gio.h>
#include <glib.h>

#define DBUS_NAME "com.unikylin.Kiran.StartMenu"
#define DBUS_OBJECT_PATH "/com/unikylin/Kiran/StartMenu"
#define DBUS_OBJECT_MAIN_INTERFACE "com.unikylin.StartMenuS"
#define DBUS_OBJECT_PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"

#define CHECK_PROXY_CALL_ERR(result, error)               \
  if (!result) {                                          \
    g_printerr("proxy call error: %s\n", error->message); \
    g_error_free(error);                                  \
    return;                                               \
  }

#define CHECK_RESULT_BOOL(result)                  \
  {                                                \
    gsize childn = g_variant_n_children(result);   \
    g_assert_true(childn == 1);                    \
    gboolean success;                              \
    g_variant_get_child(result, 0, "b", &success); \
    g_assert_true(success);                        \
  }

static void test_favorite_apps(gconstpointer data) {
  GError *error = NULL;
  GDBusProxy *proxy = G_DBUS_PROXY(data);
  GVariant *result;

  gchar *empty_strv[] = {NULL};
  result = g_dbus_proxy_call_sync(
      proxy, "org.freedesktop.DBus.Properties.Set",
      g_variant_new("(ssv)", DBUS_OBJECT_MAIN_INTERFACE, "FavoriteApps",
                    g_variant_new_strv((const gchar *const *)empty_strv, -1)),
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  CHECK_PROXY_CALL_ERR(result, error);

  const int kAppMaxLen = 10;
  gchar app[kAppMaxLen];

  for (guint i = 0; i < 10; ++i) {
    g_snprintf(app, kAppMaxLen, "apple%u", i);
    result = g_dbus_proxy_call_sync(
        proxy, "com.unikylin.StartMenuS.AddFavoriteApp",
        g_variant_new("(s)", app), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
    CHECK_PROXY_CALL_ERR(result, error);
    CHECK_RESULT_BOOL(result);
    g_variant_unref(result);
  }

  for (guint i = 0; i < 10; ++i) {
    if (i == 5 || i == 6) {
      continue;
    }
    g_snprintf(app, kAppMaxLen, "apple%u", i);
    result = g_dbus_proxy_call_sync(
        proxy, "com.unikylin.StartMenuS.DelFavoriteApp",
        g_variant_new("(s)", app), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
    CHECK_PROXY_CALL_ERR(result, error);
    CHECK_RESULT_BOOL(result);
    g_variant_unref(result);
  }

  result = g_dbus_proxy_call_sync(
      proxy, "org.freedesktop.DBus.Properties.Get",
      g_variant_new("(ss)", DBUS_OBJECT_MAIN_INTERFACE, "FavoriteApps"),
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  CHECK_PROXY_CALL_ERR(result, error);
  gsize childn = g_variant_n_children(result);
  g_assert_true(childn == 1);
  GVariant *remain_apps;
  gchar *app1;
  gchar *app2;
  g_variant_get_child(result, 0, "v", &remain_apps);
  gsize remain_app_num = g_variant_n_children(remain_apps);
  g_assert_true(remain_app_num == 2);
  g_variant_get_child(remain_apps, 0, "s", &app1);
  g_variant_get_child(remain_apps, 1, "s", &app2);
  g_assert_cmpstr(app1, ==, "apple5");
  g_assert_cmpstr(app2, ==, "apple6");
}

int main(int argc, char **argv) {
  GError *error = NULL;
  GDBusProxy *proxy = NULL;
  proxy = g_dbus_proxy_new_for_bus_sync(
      G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, NULL, DBUS_NAME,
      DBUS_OBJECT_PATH, DBUS_OBJECT_MAIN_INTERFACE, NULL, &error);

  if (!proxy) {
    g_printerr("Error creating proxy: %s\n", error->message);
    g_error_free(error);
    return 0;
  }

  g_test_init(&argc, &argv, NULL);
  g_test_add_data_func("/test-start-menu/test-favorite-apps", proxy,
                       test_favorite_apps);

  return g_test_run();
}
