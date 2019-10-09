/**
 * Copyright (C) 2012-2015 Yecheng Fu <cofyc.jackson at gmail dot com>
 * All rights reserved.
 *
 * Use of this source code is governed by a MIT-style license that can be found
 * in the LICENSE file.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "argparse.h"


static const char * prefix_skip (const char *str, const char *prefix)
{
	size_t len = strlen (prefix);
	return strncmp (str, prefix, len) ? NULL : str + len;
}


static int prefix_cmp (const char *str, const char *prefix)
{
	for (;; str++, prefix++)
	{
		if (!*prefix)
		{
			return 0;
		}
		else if (*str != *prefix)
		{
			return (unsigned char)*prefix - (unsigned char)*str;
		}
	}
}


static void argparse_error (struct argparse *self, struct argparse_option *opt, const char *reason)
{
	(void)self;
	if (opt->flags & OPT_LONG)
	{
		fprintf (stderr, "error: option `--%s` %s\n", opt->long_name, reason);
	}
	else
	{
		fprintf (stderr, "error: option `-%c` %s\n", opt->short_name, reason);
	}
	opt->flags |= OPT_ERROR;
}


static int argparse_getvalue (struct argparse *self, struct argparse_option *opt)
{
	char *s = NULL;
	if (!opt->value) {goto skipped;}
	switch (opt->type)
	{
	case ARGPARSE_OPT_BOOLEAN:
		if (opt->flags & OPT_UNSET)
		{
			*(int *)opt->value = *(int *)opt->value - 1;
		}
		else
		{
			*(int *)opt->value = *(int *)opt->value + 1;
		}
		if (*(int *)opt->value < 0)
		{
			*(int *)opt->value = 0;
		}
		break;
	case ARGPARSE_OPT_BIT:
		if (opt->flags & OPT_UNSET)
		{
			*(int *)opt->value &= ~opt->data;
		}
		else
		{
			*(int *)opt->value |= opt->data;
		}
		break;
	case ARGPARSE_OPT_STRING:
		if (self->optvalue)
		{
			*(const char **)opt->value = self->optvalue;
			self->optvalue = NULL;
		}
		else if (self->argc > 1)
		{
			self->argc--;
			*(const char **)opt->value = *++self->argv;
		}
		else
		{
			argparse_error (self, opt, "requires a value");
		}
		break;
	case ARGPARSE_OPT_INTEGER:
		errno = 0;
		if (self->optvalue)
		{
			*(int *)opt->value = strtol (self->optvalue, (char **)&s, 0);
			self->optvalue = NULL;
		}
		else if (self->argc > 1)
		{
			self->argc--;
			*(int *)opt->value = strtol (*++self->argv, (char **)&s, 0);
		}
		else
		{
			argparse_error (self, opt, "requires a value");
		}
		if (errno)
		{
			argparse_error (self, opt, strerror (errno));
		}
		if (s[0] != '\0')
		{
			argparse_error (self, opt, "expects an integer value");
		}
		break;
	case ARGPARSE_OPT_FLOAT:
		errno = 0;
		if (self->optvalue)
		{
			*(float *)opt->value = strtof(self->optvalue, (char **)&s);
			self->optvalue       = NULL;
		}
		else if (self->argc > 1)
		{
			self->argc--;
			*(float *)opt->value = strtof(*++self->argv, (char **)&s);
		}
		else
		{
			argparse_error (self, opt, "requires a value");
		}
		if (errno)
		{
			argparse_error (self, opt, strerror(errno));
		}
		if (s[0] != '\0')
		{
			argparse_error (self, opt, "expects a numerical value");
		}
		break;
	default:
		assert(0);
	}

skipped:
	if (opt->callback)
	{
		return opt->callback (self, opt);
	}

	return 0;
}


static void argparse_options_check (struct argparse *self, const struct argparse_option options [])
{
	assert (self);
	for (; options->type != ARGPARSE_OPT_END; options++)
	{
		switch (options->type)
		{
		case ARGPARSE_OPT_END:
		case ARGPARSE_OPT_BOOLEAN:
		case ARGPARSE_OPT_BIT:
		case ARGPARSE_OPT_INTEGER:
		case ARGPARSE_OPT_FLOAT:
		case ARGPARSE_OPT_STRING:
		case ARGPARSE_OPT_GROUP:
			continue;
		default:
			fprintf (stderr, "wrong option type: %d", options->type);
			break;
		}
	}
}


static int argparse_short_opt (struct argparse *self, struct argparse_option *options)
{
	assert (self);
	for (; options->type != ARGPARSE_OPT_END; options++)
	{
		if (options->short_name == *self->optvalue)
		{
			options->flags |= OPT_PRESENT;
			self->optvalue = self->optvalue[1] ? self->optvalue + 1 : NULL;
			return argparse_getvalue (self, options);
		}
	}
	return ARGPARSE_UNKOWN_OPTION;
}


static int argparse_long_opt (struct argparse *self, struct argparse_option *options)
{
	assert (self);
	for (; options->type != ARGPARSE_OPT_END; options++)
	{
		const char *rest;
		if (!options->long_name) {continue;}
		rest = prefix_skip (self->argv[0] + 2, options->long_name);
		if (!rest)
		{
			// negation disabled?
			if (options->flags & OPT_NONEG)
			{
				continue;
			}
			// only OPT_BOOLEAN/OPT_BIT supports negation
			if (options->type != ARGPARSE_OPT_BOOLEAN && options->type != ARGPARSE_OPT_BIT)
			{
				continue;
			}

			if (prefix_cmp (self->argv[0] + 2, "no-"))
			{
				continue;
			}
			rest = prefix_skip (self->argv[0] + 2 + 3, options->long_name);
			if (!rest) {continue;}
			options->flags |= OPT_UNSET;
		}
		if (*rest)
		{
			if (*rest != '=') {continue;}
			self->optvalue = rest + 1;
		}
		options->flags |= OPT_LONG;
		return argparse_getvalue (self, options);
	}
	return ARGPARSE_UNKOWN_OPTION;
}


int argparse_init (struct argparse *self, struct argparse_option *options, const char *const usages [], int flags)
{
	assert (self);
	memset (self, 0, sizeof(*self));
	self->options = options;
	self->usages = usages;
	self->flags = flags;
	self->description = NULL;
	self->epilog = NULL;
	return 0;
}


void argparse_describe (struct argparse *self, const char *description, const char *epilog)
{
	assert (self);
	self->description = description;
	self->epilog = epilog;
}


int argparse_parse (struct argparse *self, int argc, const char *argv [])
{
	assert (self);
	self->argc = argc - 1;
	self->argv = argv + 1;
	self->out = argv;

	argparse_options_check (self, self->options);

	for (; self->argc; self->argc--, self->argv++)
	{
		const char *arg = self->argv[0];
		if (arg[0] != '-' || !arg[1])
		{
			if (self->flags & ARGPARSE_STOP_AT_NON_OPTION)
			{
				goto end;
			}
			// if it's not option or is a single char '-', copy verbatim
			self->out[self->cpidx++] = self->argv[0];
			continue;
		}
		// short option
		if (arg[1] != '-')
		{
			self->optvalue = arg + 1;
			switch (argparse_short_opt (self, self->options))
			{
			case -1: break;
			case ARGPARSE_UNKOWN_OPTION: goto unknown;
			}
			while (self->optvalue)
			{
				switch (argparse_short_opt (self, self->options))
				{
				case -1: break;
				case ARGPARSE_UNKOWN_OPTION: goto unknown;
				}
			}
			continue;
		}
		// if '--' presents
		if (!arg[2])
		{
			self->argc--;
			self->argv++;
			break;
		}
		// long option
		switch (argparse_long_opt (self, self->options))
		{
		case -1: break;
		case ARGPARSE_UNKOWN_OPTION: goto unknown;
		}
		continue;
unknown:
		fprintf (stdout, "error: unknown option `%s`\n", self->argv[0]);
		argparse_usage (self);
		exit (1);
	}

end:
	memmove (self->out + self->cpidx, self->argv, (size_t)self->argc * sizeof(*self->out));
	self->out[self->cpidx + self->argc] = NULL;
	return self->cpidx + self->argc;
}


void argparse_usage1 (const char *const *usages)
{
	if (usages)
	{
		fprintf (stdout, "Usage: %s\n", *usages++);
		while (*usages && **usages)
		{
			fprintf (stdout, "   or: %s\n", *usages++);
		}
	}
	else
	{
		fprintf (stdout, "Usage:\n");
	}
}


void argparse_usage (struct argparse *self)
{
	assert (self);
	argparse_usage1 (self->usages);
	if (self->description)
	{
		fprintf (stdout, "%s\n", self->description);
	}
	fputc ('\n', stdout);
	const struct argparse_option *options;
	// figure out best width
	int usage_opts_width = 0;
	int len;
	options = self->options;
	for (; options->type != ARGPARSE_OPT_END; options++)
	{
		len = 0;
		if ((options)->short_name)
		{
			len += 2;
		}
		if ((options)->short_name && (options)->long_name)
		{
			// separator ", "
			len += 2;
		}
		if ((options)->long_name)
		{
			len += strlen ((options)->long_name) + 2;
		}
		if (options->type == ARGPARSE_OPT_INTEGER)
		{
			len += strlen ("=<int>");
		}
		if (options->type == ARGPARSE_OPT_FLOAT)
		{
			len += strlen ("=<flt>");
		}
		else if (options->type == ARGPARSE_OPT_STRING)
		{
			len += strlen ("=<str>");
		}
		len = (len + 3) - ((len + 3) & 3);
		if (usage_opts_width < len)
		{
			usage_opts_width = len;
		}
	}
	usage_opts_width += 4;      // 4 spaces prefix

	options = self->options;
	for (; options->type != ARGPARSE_OPT_END; options++)
	{
		int pos = 0;
		int pad    = 0;
		if (options->type == ARGPARSE_OPT_GROUP)
		{
			fputc('\n', stdout);
			fprintf(stdout, "%s", options->help);
			fputc('\n', stdout);
			continue;
		}
		pos = fprintf (stdout, "    ");
		if (options->short_name)
		{
			pos += fprintf (stdout, "-%c", options->short_name);
		}
		if (options->long_name && options->short_name)
		{
			pos += fprintf (stdout, ", ");
		}
		if (options->long_name)
		{
			pos += fprintf (stdout, "--%s", options->long_name);
		}
		if (options->type == ARGPARSE_OPT_INTEGER)
		{
			pos += fprintf (stdout, "=<int>");
		}
		else if (options->type == ARGPARSE_OPT_FLOAT)
		{
			pos += fprintf (stdout, "=<flt>");
		}
		else if (options->type == ARGPARSE_OPT_STRING)
		{
			pos += fprintf (stdout, "=<str>");
		}
		if (pos <= usage_opts_width)
		{
			pad = usage_opts_width - pos;
		}
		else
		{
			fputc ('\n', stdout);
			pad = usage_opts_width;
		}
		fprintf (stdout, "%*s%s\n", pad + 2, "", options->help);
	}

	// print epilog
	if (self->epilog)
	{
		fprintf (stdout, "%s\n", self->epilog);
	}
}
