X_LIBS		+= libxlibc.a libsconf.a

X_INCDIRS	+= bash/ bash/include bash/lib

Program = bash
Version = 5.1
Machine = x86
OS = bookos
VENDOR = pc
MACHTYPE = x86-pc-bookos
localedir = /usr/local
PACKAGE = bash
prefix = /usr/local
aliaspath = $(prefix)/data/locale

X_CFLAGS	+= -w -DPROGRAM='"$(Program)"' -DCONF_HOSTTYPE='"$(Machine)"'\
			-DCONF_OSTYPE='"$(OS)"' -DCONF_MACHTYPE='"$(MACHTYPE)"' \
			-DCONF_VENDOR='"$(VENDOR)"' -DLOCALEDIR='"$(localedir)"' -DPACKAGE='"$(PACKAGE)"' \
			-DHAVE_CONFIG_H -DLIBDIR=\"$(prefix)/libdata\" -DLOCALE_ALIAS_PATH=\"$(aliaspath)\" \
			-D__XBOOK__ -DIN_LIBINTL
			
NAME		:= bash
SRC			+= shell.c eval.c general.c make_cmd.c print_cmd.c y.tab.c \
			dispose_cmd.c execute_cmd.c variables.c version.c \
			expr.c copy_cmd.c flags.c subst.c hashcmd.c hashlib.c mailcheck.c \
			test.c trap.c alias.c nojobs.c braces.c \
			input.c bashhist.c array.c arrayfunc.c assoc.c sig.c pathexp.c \
			unwind_prot.c siglist.c bashline.c bracecomp.c error.c \
			list.c stringlib.c locale.c findcmd.c redir.c \
			pcomplete.c pcomplib.c syntax.c xmalloc.c

SRC			+= builtins/*.c

SRC			+= lib/glob/glob.c
SRC			+= lib/glob/strmatch.c
SRC			+= lib/glob/smatch.c
SRC			+= lib/glob/xmbsrtowcs.c
SRC			+= lib/glob/gmisc.c
SRC			+= lib/tilde/tilde.c

SRC			+= lib/sh/*.c

SRC			+= lib/termcap/termcap.c
SRC			+= lib/termcap/tparm.c
SRC			+= lib/intl/bindtextdom.c
SRC			+= lib/intl/dcgettext.c
SRC			+= lib/intl/dgettext.c
SRC			+= lib/intl/gettext.c
SRC			+= lib/intl/finddomain.c
SRC			+= lib/intl/loadmsgcat.c
SRC			+= lib/intl/localealias.c
SRC			+= lib/intl/textdomain.c
SRC			+= lib/intl/l10nflist.c
SRC			+= lib/intl/explodename.c
SRC			+= lib/intl/dcigettext.c
SRC			+= lib/intl/dcngettext.c
SRC			+= lib/intl/dngettext.c
SRC			+= lib/intl/ngettext.c
SRC			+= lib/intl/plural.c
SRC			+= lib/intl/plural-exp.c
SRC			+= lib/intl/localcharset.c
SRC			+= lib/intl/relocatable.c
SRC			+= lib/intl/localename.c
SRC			+= lib/intl/log.c
SRC			+= lib/intl/osdep.c
SRC			+= lib/intl/intl-compat.c

SRC			+= 	lib/readline/readline.c lib/readline/funmap.c lib/readline/keymaps.c \
				lib/readline/vi_mode.c lib/readline/parens.c lib/readline/rltty.c \
				lib/readline/complete.c lib/readline/bind.c lib/readline/isearch.c \
				lib/readline/display.c lib/readline/signals.c \
				lib/readline/util.c lib/readline/kill.c \
				lib/readline/undo.c lib/readline/macro.c lib/readline/input.c \
				lib/readline/callback.c lib/readline/terminal.c \
				lib/readline/history.c lib/readline/histsearch.c lib/readline/histexpand.c \
				lib/readline/histfile.c lib/readline/nls.c lib/readline/search.c \
				lib/readline/savestring.c \
				lib/readline/text.c lib/readline/misc.c lib/readline/compat.c \
				lib/readline/colors.c lib/readline/parse-colors.c \
				lib/readline/mbutil.c

define CUSTOM_TARGET_CMD
echo [APP] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) -o $@ $(patsubst %, -L%, $(X_LIBDIRS)) --start-group $(patsubst %, -l:%, $(X_LIBS)) --end-group; \
cp $@ $(srctree)/../develop/rom/bin
endef
