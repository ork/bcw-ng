#include <glib.h>

#define GETTEXT_PACKAGE "bcw-ng"

static GOptionContext* context = NULL;

static gchar* model = NULL;
static gchar* definition = NULL;
static gchar** remaining = NULL;
static gboolean verbose = FALSE;
static gchar* commandline = NULL;

void args_register(void)
{
    static GOptionEntry entries[] = {
        { "model", 'm', 0, G_OPTION_ARG_STRING, &model, "Printer model", "FAMILY" },
        { "definition", 'd', 0, G_OPTION_ARG_FILENAME, &definition, "Printer Postscript Description file", "PPDFILE" },
        { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Verbose output for debugging purpose", NULL },
        { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &remaining, "Command line for brprintconf", NULL },
        { NULL }
    };

    context = g_option_context_new("-- BRPRINTCONF COMMAND LINE");
    g_option_context_set_description(context, "Report any bugs at <https://github.com/ork/bcw-ng>.");

    g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);
}

void args_execute(int argc, char* argv[])
{
    GError* error = NULL;

    g_option_context_parse(context, &argc, &argv, &error);
    g_option_context_free(context);

    if (error != NULL) {
        g_error("option parsing failed: %s\n", error->message);
    }

    /* Change debug level */
    if (verbose) {
        gboolean applied = g_setenv("G_MESSAGES_DEBUG", "all", TRUE);
        if (!applied) {
            g_error("Verbosity level could not be changed");
        }
    }

    /* Use a default PPD file if not specified */
    if (!definition) {
        definition = g_strdup_printf("/usr/share/cups/model/%s.ppd");
    }

    if (!g_file_test(definition, G_FILE_TEST_EXISTS)) {
        g_error("PPD file could not be located: %s");
    }

    /* Concatenate remaining argument as a single string to feed brprintconfig */
    if (remaining != NULL) {
        commandline = g_strjoinv(" ", remaining);
        g_strfreev(remaining);
    } else {
        commandline = g_strdup("NULL COMMAND LINE");
    }

    g_debug("Using printer model: %s", model);
    g_debug("Using PPD file: %s", definition);
    g_debug("Using brprintconf command line: %s", commandline);
}

int main(int argc, char* argv[])
{
    args_register();
    args_execute(argc, argv);

    return 0;
}
