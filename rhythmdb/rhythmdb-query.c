/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 *  arch-tag: Implementation of RhythmDB - Rhythmbox backend queryable database
 *
 *  Copyright (C) 2003,2004 Colin Walters <walters@gnome.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.
 *
 */

#include <config.h>

#include <string.h>

#include <glib.h>
#include <glib-object.h>
#include <gobject/gvaluecollector.h>

#include "rhythmdb.h"
#include "rhythmdb-private.h"
#include "rb-util.h"

#define RB_PARSE_CONJ (xmlChar *) "conjunction"
#define RB_PARSE_SUBQUERY (xmlChar *) "subquery"
#define RB_PARSE_LIKE (xmlChar *) "like"
#define RB_PARSE_PROP (xmlChar *) "prop"
#define RB_PARSE_NOT_LIKE (xmlChar *) "not-like"
#define RB_PARSE_PREFIX (xmlChar *) "prefix"
#define RB_PARSE_SUFFIX (xmlChar *) "suffix"
#define RB_PARSE_EQUALS (xmlChar *) "equals"
#define RB_PARSE_DISJ (xmlChar *) "disjunction"
#define RB_PARSE_GREATER (xmlChar *) "greater"
#define RB_PARSE_LESS (xmlChar *) "less"
#define RB_PARSE_CURRENT_TIME_WITHIN (xmlChar *) "current-time-within"
#define RB_PARSE_CURRENT_TIME_NOT_WITHIN (xmlChar *) "current-time-not-within"
#define RB_PARSE_YEAR_EQUALS RB_PARSE_EQUALS
#define RB_PARSE_YEAR_GREATER RB_PARSE_GREATER
#define RB_PARSE_YEAR_LESS RB_PARSE_LESS
/**
 * rhythmdb_query_copy:
 * @array: the query to copy.
 *
 * Creates a copy of a query.
 *
 * Return value: a copy of the passed query. It must be freed with rhythmdb_query_free()
 **/
GPtrArray *
rhythmdb_query_copy (GPtrArray *array)
{
	GPtrArray *ret;

	if (!array)
		return NULL;
	
	ret = g_ptr_array_sized_new (array->len);
	rhythmdb_query_concatenate (ret, array);

	return ret;
}

void
rhythmdb_query_concatenate (GPtrArray *query1, GPtrArray *query2)
{
	guint i;

	g_assert (query2);
	if (!query2)
		return;
	
	for (i = 0; i < query2->len; i++) {
		RhythmDBQueryData *data = g_ptr_array_index (query2, i);
		RhythmDBQueryData *new_data = g_new0 (RhythmDBQueryData, 1);
		new_data->type = data->type;
		new_data->propid = data->propid;
		if (data->val) {
			new_data->val = g_new0 (GValue, 1);
			g_value_init (new_data->val, G_VALUE_TYPE (data->val));
			g_value_copy (data->val, new_data->val);
		}
		if (data->subquery)
			new_data->subquery = rhythmdb_query_copy (data->subquery);
		g_ptr_array_add (query1, new_data);
	}
}


GPtrArray *
rhythmdb_query_parse_valist (RhythmDB *db, va_list args)
{
	RhythmDBQueryType query;
	GPtrArray *ret = g_ptr_array_new ();
	char *error;
	
	while ((query = va_arg (args, RhythmDBQueryType)) != RHYTHMDB_QUERY_END) {
		RhythmDBQueryData *data = g_new0 (RhythmDBQueryData, 1);
		data->type = query;
		switch (query) {
		case RHYTHMDB_QUERY_DISJUNCTION:
			break;
		case RHYTHMDB_QUERY_SUBQUERY:
			data->subquery = va_arg (args, GPtrArray *);
			break;
		case RHYTHMDB_QUERY_PROP_EQUALS:
		case RHYTHMDB_QUERY_PROP_LIKE:
		case RHYTHMDB_QUERY_PROP_NOT_LIKE:
		case RHYTHMDB_QUERY_PROP_PREFIX:
		case RHYTHMDB_QUERY_PROP_SUFFIX:
		case RHYTHMDB_QUERY_PROP_GREATER:
		case RHYTHMDB_QUERY_PROP_LESS:
		case RHYTHMDB_QUERY_PROP_CURRENT_TIME_WITHIN:
		case RHYTHMDB_QUERY_PROP_CURRENT_TIME_NOT_WITHIN:
		case RHYTHMDB_QUERY_PROP_YEAR_EQUALS:
		case RHYTHMDB_QUERY_PROP_YEAR_GREATER:
		case RHYTHMDB_QUERY_PROP_YEAR_LESS:  
			data->propid = va_arg (args, guint);
			data->val = g_new0 (GValue, 1);
			g_value_init (data->val, rhythmdb_get_property_type (db, data->propid));
			G_VALUE_COLLECT (data->val, args, 0, &error);
			break;
		case RHYTHMDB_QUERY_END:
			g_assert_not_reached ();
			break;
		}
		g_ptr_array_add (ret, data);
	}
	return ret;
}

/**
 * rhythmdb_query_parse:
 * @db: a #RhythmDB instance
 *
 * Creates a query from a list of criteria.
 *
 * Most criteria consists of an operator (#RhythmDBQueryType),
 * a property (#RhythmDBPropType) and the data to compare with. An entry
 * matches a criteria if the operator returns true with the value of the
 * entries property as the first argument, and the given data as the second
 * argument.
 *
 * Three types criteria are special. Passing RHYTHMDB_QUERY_END indicates the
 * end of the list of criteria, and must be the last passes parameter.
 *
 * The second special criteria is a subquery which is defined by passing
 * RHYTHMDB_QUERY_SUBQUERY, followed by a query (#GPtrArray). An entry will
 * match a subquery criteria if it matches all criteria in the subquery.
 *
 * The third special criteria is a disjunction which is defined by passing
 * RHYTHMDB_QUERY_DISJUNCTION, which will make an entry match the query if
 * it matches the criteria before the disjunction, the criteria after the
 * disjunction, or both.
 *
 * Example:
 * 	rhythmdb_query_parse (db,
 * 		RHYTHMDB_QUERY_SUBQUERY, subquery,
 * 		RHYTHMDB_QUERY_DISJUNCTION
 * 		RHYTHMDB_QUERY_PROP_LIKE, RHYTHMDB_PROP_TITLE, "cat",
 *		RHYTHMDB_QUERY_DISJUNCTION
 *		RHYTHMDB_QUERY_PROP_GREATER, RHYTHMDB_PROP_RATING, 2.5,
 *		RHYTHMDB_QUERY_PROP_LESS, RHYTHMDB_PROP_PLAY_COUNT, 10,
 * 		RHYTHMDB_QUERY_END);
 *
 * 	will create a query that matches entries:
 * 	a) that match the query "subquery", or
 * 	b) that have "cat" in their title, or
 * 	c) have a rating of at least 2.5, and a play count of at most 10
 * 
 * Returns: a the newly created query. It must be freed with rhythmdb_query_free()
 **/
GPtrArray *
rhythmdb_query_parse (RhythmDB *db, ...)
{
	GPtrArray *ret;
	va_list args;

	va_start (args, db);

	ret = rhythmdb_query_parse_valist (db, args);

	va_end (args);

	return ret;
}


/**
 * rhythmdb_query_append:
 * @db: a #RhythmDB instance
 * @query: a query.
 *
 * Appends new criteria to the query @query.
 *
 * The list of criteria must be in the same format as for rhythmdb_query_parse,
 * and ended by RHYTHMDB_QUERY_END.
 **/
void
rhythmdb_query_append (RhythmDB *db, GPtrArray *query, ...)
{
	va_list args;
	guint i;
	GPtrArray *new = g_ptr_array_new ();

	va_start (args, query);

	new = rhythmdb_query_parse_valist (db, args);

	for (i = 0; i < new->len; i++)
		g_ptr_array_add (query, g_ptr_array_index (new, i));

	g_ptr_array_free (new, TRUE);

	va_end (args);
}

/**
 * rhythmdb_query_free:
 * @query: a query.
 *
 * Frees the query @query
 **/
void
rhythmdb_query_free (GPtrArray *query)
{
	guint i;

	if (query == NULL)
		return;
	
	for (i = 0; i < query->len; i++) {
		RhythmDBQueryData *data = g_ptr_array_index (query, i);
		switch (data->type) {
		case RHYTHMDB_QUERY_DISJUNCTION:
			break;
		case RHYTHMDB_QUERY_SUBQUERY:
			rhythmdb_query_free (data->subquery);
			break;
		case RHYTHMDB_QUERY_PROP_EQUALS:
		case RHYTHMDB_QUERY_PROP_LIKE:
		case RHYTHMDB_QUERY_PROP_NOT_LIKE:
		case RHYTHMDB_QUERY_PROP_PREFIX:
		case RHYTHMDB_QUERY_PROP_SUFFIX:
		case RHYTHMDB_QUERY_PROP_GREATER:
		case RHYTHMDB_QUERY_PROP_LESS:
		case RHYTHMDB_QUERY_PROP_CURRENT_TIME_WITHIN:
		case RHYTHMDB_QUERY_PROP_CURRENT_TIME_NOT_WITHIN:
		case RHYTHMDB_QUERY_PROP_YEAR_EQUALS:
		case RHYTHMDB_QUERY_PROP_YEAR_GREATER:
		case RHYTHMDB_QUERY_PROP_YEAR_LESS:  
			g_value_unset (data->val);
			g_free (data->val);
			break;
		case RHYTHMDB_QUERY_END:
			g_assert_not_reached ();
			break;
		}
		g_free (data);
	}

	g_ptr_array_free (query, TRUE);
}

static char *
entry_type_to_string (RhythmDBEntryType type)
{
	if (type == RHYTHMDB_ENTRY_TYPE_SONG) {
		return g_strdup ("0");
	} else if (type == RHYTHMDB_ENTRY_TYPE_IRADIO_STATION) {
		return g_strdup ("1");
	} else if (type == RHYTHMDB_ENTRY_TYPE_PODCAST_POST) {
		return g_strdup ("2");
	} else if (type == RHYTHMDB_ENTRY_TYPE_PODCAST_FEED) {
		return g_strdup ("3");
	}
	g_assert_not_reached ();
}

static RhythmDBEntryType
entry_type_from_uint (unsigned int type) 
{
	switch (type) {
	case 0:
		return RHYTHMDB_ENTRY_TYPE_SONG;
	case 1:
		return RHYTHMDB_ENTRY_TYPE_IRADIO_STATION;
	case 2:
		return RHYTHMDB_ENTRY_TYPE_PODCAST_POST;
	case 3:
		return RHYTHMDB_ENTRY_TYPE_PODCAST_FEED;
	default:
		g_assert_not_reached ();
	}

	return RHYTHMDB_ENTRY_TYPE_INVALID;
}


static void
write_encoded_gvalue (xmlNodePtr node,
		      GValue *val)
{
	char *strval;
	xmlChar *quoted;

	switch (G_VALUE_TYPE (val)) {
	case G_TYPE_STRING:
		strval = g_value_dup_string (val);
		break;
	case G_TYPE_BOOLEAN:
		strval = g_strdup_printf ("%d", g_value_get_boolean (val));
		break;
	case G_TYPE_INT:
		strval = g_strdup_printf ("%d", g_value_get_int (val));
		break;
	case G_TYPE_LONG:
		strval = g_strdup_printf ("%ld", g_value_get_long (val));
		break;
	case G_TYPE_ULONG:
		strval = g_strdup_printf ("%lu", g_value_get_ulong (val));
		break;
	case G_TYPE_UINT64:
		strval = g_strdup_printf ("%" G_GUINT64_FORMAT, g_value_get_uint64 (val));
		break;
	case G_TYPE_FLOAT:
		strval = g_strdup_printf ("%f", g_value_get_float (val));
		break;
	case G_TYPE_DOUBLE:
		strval = g_strdup_printf ("%f", g_value_get_double (val));
		break;
	case G_TYPE_POINTER:
		strval = entry_type_to_string (g_value_get_pointer (val));
		break;
	default:
		g_assert_not_reached ();
		strval = NULL;
		break;
	}

	quoted = xmlEncodeEntitiesReentrant (NULL, BAD_CAST strval);
	g_free (strval);
	xmlNodeSetContent (node, quoted);
	g_free (quoted);
}

static void
read_encoded_property (RhythmDB *db,
		       xmlNodePtr node,
		       guint propid,
		       GValue *val)
{
	char *content;

	g_value_init (val, rhythmdb_get_property_type (db, propid));

	content = (char *)xmlNodeGetContent (node);
	
	switch (G_VALUE_TYPE (val)) {
	case G_TYPE_STRING:
		g_value_set_string (val, content);
		break;
	case G_TYPE_BOOLEAN:
		g_value_set_boolean (val, g_ascii_strtoull (content, NULL, 10));
		break;
	case G_TYPE_ULONG:
		g_value_set_ulong (val, g_ascii_strtoull (content, NULL, 10));
		break;
	case G_TYPE_UINT64:
		g_value_set_uint64 (val, g_ascii_strtoull (content, NULL, 10));
		break;
	case G_TYPE_DOUBLE:
		g_value_set_double (val, g_ascii_strtod (content, NULL));
		break;
	case G_TYPE_POINTER:
		if (propid == RHYTHMDB_PROP_TYPE) {
			RhythmDBEntryType entry_type;
			entry_type = entry_type_from_uint (g_ascii_strtoull (content, NULL, 10));
			if (entry_type != RHYTHMDB_ENTRY_TYPE_INVALID) {
				g_value_set_pointer (val, entry_type);
				break;
			} else {
				g_warning ("Unexpected entry type");
				/* Fall through */
			}
		}
		/* Falling through on purpose to get an assert for unexpected 
		 * cases 
		 */
	default:
		g_warning ("Attempt to read '%s' of unhandled type %s", 
			   rhythmdb_nice_elt_name_from_propid (db, propid),
			   g_type_name (G_VALUE_TYPE (val)));
		g_assert_not_reached ();
		break;
	}

	g_free (content);
}

void
rhythmdb_query_serialize (RhythmDB *db, GPtrArray *query,
			  xmlNodePtr parent)
{
	guint i;
	xmlNodePtr node = xmlNewChild (parent, NULL, RB_PARSE_CONJ, NULL);
	xmlNodePtr subnode;

	for (i = 0; i < query->len; i++) {
		RhythmDBQueryData *data = g_ptr_array_index (query, i);
		
		switch (data->type) {
		case RHYTHMDB_QUERY_SUBQUERY:
			subnode = xmlNewChild (node, NULL, RB_PARSE_SUBQUERY, NULL);
			rhythmdb_query_serialize (db, data->subquery, subnode);
			break;
		case RHYTHMDB_QUERY_PROP_LIKE:
			subnode = xmlNewChild (node, NULL, RB_PARSE_LIKE, NULL);
			xmlSetProp (subnode, RB_PARSE_PROP, rhythmdb_nice_elt_name_from_propid (db, data->propid));
			write_encoded_gvalue (subnode, data->val);
			break;
		case RHYTHMDB_QUERY_PROP_NOT_LIKE:
			subnode = xmlNewChild (node, NULL, RB_PARSE_NOT_LIKE, NULL);
			xmlSetProp (subnode, RB_PARSE_PROP, rhythmdb_nice_elt_name_from_propid (db, data->propid));
			write_encoded_gvalue (subnode, data->val);
			break;
		case RHYTHMDB_QUERY_PROP_PREFIX:
			subnode = xmlNewChild (node, NULL, RB_PARSE_PREFIX, NULL);
			xmlSetProp (subnode, RB_PARSE_PROP, rhythmdb_nice_elt_name_from_propid (db, data->propid));
			write_encoded_gvalue (subnode, data->val);
			break;
		case RHYTHMDB_QUERY_PROP_SUFFIX:
			subnode = xmlNewChild (node, NULL, RB_PARSE_SUFFIX, NULL);
			xmlSetProp (subnode, RB_PARSE_PROP, rhythmdb_nice_elt_name_from_propid (db, data->propid));
			write_encoded_gvalue (subnode, data->val);
			break;
		case RHYTHMDB_QUERY_PROP_EQUALS:
			subnode = xmlNewChild (node, NULL, RB_PARSE_EQUALS, NULL);
			xmlSetProp (subnode, RB_PARSE_PROP, rhythmdb_nice_elt_name_from_propid (db, data->propid));
			write_encoded_gvalue (subnode, data->val);
			break;
		case RHYTHMDB_QUERY_PROP_YEAR_EQUALS:
			subnode = xmlNewChild (node, NULL, RB_PARSE_YEAR_EQUALS, NULL);
			xmlSetProp (subnode, RB_PARSE_PROP, rhythmdb_nice_elt_name_from_propid (db, data->propid));
			write_encoded_gvalue (subnode, data->val);
			break;
		case RHYTHMDB_QUERY_DISJUNCTION:
			subnode = xmlNewChild (node, NULL, RB_PARSE_DISJ, NULL);
			break;
		case RHYTHMDB_QUERY_END:
			break;
		case RHYTHMDB_QUERY_PROP_GREATER:
			subnode = xmlNewChild (node, NULL, RB_PARSE_GREATER, NULL);
			xmlSetProp (subnode, RB_PARSE_PROP, rhythmdb_nice_elt_name_from_propid (db, data->propid));
			write_encoded_gvalue (subnode, data->val);
			break;
		case RHYTHMDB_QUERY_PROP_YEAR_GREATER:
			subnode = xmlNewChild (node, NULL, RB_PARSE_YEAR_GREATER, NULL);
			xmlSetProp (subnode, RB_PARSE_PROP, rhythmdb_nice_elt_name_from_propid (db, data->propid));
			write_encoded_gvalue (subnode, data->val);
			break;
		case RHYTHMDB_QUERY_PROP_LESS:
			subnode = xmlNewChild (node, NULL, RB_PARSE_LESS, NULL);
			xmlSetProp (subnode, RB_PARSE_PROP, rhythmdb_nice_elt_name_from_propid (db, data->propid));
			write_encoded_gvalue (subnode, data->val);
			break;
		case RHYTHMDB_QUERY_PROP_YEAR_LESS:  
			subnode = xmlNewChild (node, NULL, RB_PARSE_YEAR_LESS, NULL);
			xmlSetProp (subnode, RB_PARSE_PROP, rhythmdb_nice_elt_name_from_propid (db, data->propid));
			write_encoded_gvalue (subnode, data->val);
			break;
		case RHYTHMDB_QUERY_PROP_CURRENT_TIME_WITHIN:
			subnode = xmlNewChild (node, NULL, RB_PARSE_CURRENT_TIME_WITHIN, NULL);
			xmlSetProp (subnode, RB_PARSE_PROP, rhythmdb_nice_elt_name_from_propid (db, data->propid));
			write_encoded_gvalue (subnode, data->val);
			break;
		case RHYTHMDB_QUERY_PROP_CURRENT_TIME_NOT_WITHIN:
			subnode = xmlNewChild (node, NULL, RB_PARSE_CURRENT_TIME_NOT_WITHIN, NULL);
			xmlSetProp (subnode, RB_PARSE_PROP, rhythmdb_nice_elt_name_from_propid (db, data->propid));
			write_encoded_gvalue (subnode, data->val);
			break;
		}		
	}
}

GPtrArray *
rhythmdb_query_deserialize (RhythmDB *db, xmlNodePtr parent)
{
	GPtrArray *query = g_ptr_array_new ();
	xmlNodePtr child;

	g_assert (!xmlStrcmp (parent->name, RB_PARSE_CONJ));
	
	for (child = parent->children; child; child = child->next) {
		RhythmDBQueryData *data;

		if (xmlNodeIsText (child))
			continue;

		data = g_new0 (RhythmDBQueryData, 1);

		if (!xmlStrcmp (child->name, RB_PARSE_SUBQUERY)) {
			xmlNodePtr subquery;
			data->type = RHYTHMDB_QUERY_SUBQUERY;
			subquery = child->children;
			while (xmlNodeIsText (subquery))
				subquery = subquery->next;
			
			data->subquery = rhythmdb_query_deserialize (db, subquery);
		} else if (!xmlStrcmp (child->name, RB_PARSE_DISJ)) {
			data->type = RHYTHMDB_QUERY_DISJUNCTION;
		} else if (!xmlStrcmp (child->name, RB_PARSE_LIKE)) {
			data->type = RHYTHMDB_QUERY_PROP_LIKE;
		} else if (!xmlStrcmp (child->name, RB_PARSE_NOT_LIKE)) {
			data->type = RHYTHMDB_QUERY_PROP_NOT_LIKE;
		} else if (!xmlStrcmp (child->name, RB_PARSE_PREFIX)) {
			data->type = RHYTHMDB_QUERY_PROP_PREFIX;
		} else if (!xmlStrcmp (child->name, RB_PARSE_SUFFIX)) {
			data->type = RHYTHMDB_QUERY_PROP_SUFFIX;
		} else if (!xmlStrcmp (child->name, RB_PARSE_EQUALS)) {
			if (!xmlStrcmp(xmlGetProp(child, RB_PARSE_PROP), 
				       (xmlChar *)"date")) 
				data->type = RHYTHMDB_QUERY_PROP_YEAR_EQUALS;
			else 
				data->type = RHYTHMDB_QUERY_PROP_EQUALS;
		} else if (!xmlStrcmp (child->name, RB_PARSE_GREATER)) {
			if (!xmlStrcmp(xmlGetProp(child, RB_PARSE_PROP), 
				       (xmlChar *)"date")) 
				data->type = RHYTHMDB_QUERY_PROP_YEAR_GREATER;
			else 
				data->type = RHYTHMDB_QUERY_PROP_GREATER;
		} else if (!xmlStrcmp (child->name, RB_PARSE_LESS)) {
			if (!xmlStrcmp(xmlGetProp(child, RB_PARSE_PROP), 
				       (xmlChar *)"date")) 
				data->type = RHYTHMDB_QUERY_PROP_YEAR_LESS;
			else 
				data->type = RHYTHMDB_QUERY_PROP_LESS;
		} else if (!xmlStrcmp (child->name, RB_PARSE_CURRENT_TIME_WITHIN)) {
			data->type = RHYTHMDB_QUERY_PROP_CURRENT_TIME_WITHIN;
		} else if (!xmlStrcmp (child->name, RB_PARSE_CURRENT_TIME_NOT_WITHIN)) {
			data->type = RHYTHMDB_QUERY_PROP_CURRENT_TIME_NOT_WITHIN;
		} else
 			g_assert_not_reached ();

		if (!xmlStrcmp (child->name, RB_PARSE_LIKE)
		    || !xmlStrcmp (child->name, RB_PARSE_NOT_LIKE)
		    || !xmlStrcmp (child->name, RB_PARSE_PREFIX)
		    || !xmlStrcmp (child->name, RB_PARSE_SUFFIX)
		    || !xmlStrcmp (child->name, RB_PARSE_EQUALS)
		    || !xmlStrcmp (child->name, RB_PARSE_GREATER)
		    || !xmlStrcmp (child->name, RB_PARSE_LESS)
		    || !xmlStrcmp (child->name, RB_PARSE_YEAR_EQUALS)
		    || !xmlStrcmp (child->name, RB_PARSE_YEAR_GREATER)
		    || !xmlStrcmp (child->name, RB_PARSE_YEAR_LESS)
		    || !xmlStrcmp (child->name, RB_PARSE_CURRENT_TIME_WITHIN)
		    || !xmlStrcmp (child->name, RB_PARSE_CURRENT_TIME_NOT_WITHIN)) {
			xmlChar *propstr = xmlGetProp (child, RB_PARSE_PROP);
			gint propid = rhythmdb_propid_from_nice_elt_name (db, propstr);
			g_free (propstr);

			g_assert (propid >= 0 && propid < RHYTHMDB_NUM_PROPERTIES);

			data->propid = propid;
			data->val = g_new0 (GValue, 1);

			read_encoded_property (db, child, data->propid, data->val);
		} 

		g_ptr_array_add (query, data);
	}

	return query;
}

/**
 * This is used to "process" queries, before using them. It is mainly used to two things:
 *
 * 1) performing expensive data transformations once per query, rather than
 *    once per entry we try to match against. e.g. RHYTHMDB_PROP_SEARCH_MATCH
 *
 * 2) defining criteria in terms of other lower-level ones that the db backend
 *    actually implements. e.g. RHYTHMDB_QUERY_YEAR_*
 **/

void
rhythmdb_query_preprocess (RhythmDB *db, GPtrArray *query)
{
	int i;	

	if (query == NULL)
		return;

	for (i = 0; i < query->len; i++) {
		RhythmDBQueryData *data = g_ptr_array_index (query, i);
		gboolean restart_criteria = FALSE;
		
		if (data->subquery) {
			rhythmdb_query_preprocess (db, data->subquery);
		} else switch (data->propid) {
			case RHYTHMDB_PROP_TITLE_FOLDED:
			case RHYTHMDB_PROP_GENRE_FOLDED:
			case RHYTHMDB_PROP_ARTIST_FOLDED:
			case RHYTHMDB_PROP_ALBUM_FOLDED:
			{
				/* as we are matching against a folded property, the string needs to also be folded */
				const char *orig = g_value_get_string (data->val);
				char *folded = rb_search_fold (orig);

				g_value_reset (data->val);
				g_value_take_string (data->val, folded);
				break;
			}

			case RHYTHMDB_PROP_SEARCH_MATCH:
			{
				const char *orig = g_value_get_string (data->val);
				char *folded = rb_search_fold (orig);
				char **words = rb_string_split_words (folded);

				g_free (folded);
				g_value_unset (data->val);
				g_value_init (data->val, G_TYPE_STRV);
				g_value_take_boxed (data->val, words);
				break;
			}

			case RHYTHMDB_PROP_DATE:
			{
				GDate date = {0,};
				gulong search_date;
				gulong begin;
				gulong end;
				gulong year;

				search_date = g_value_get_ulong (data->val);
				g_date_set_julian (&date, search_date);
				year = g_date_get_year (&date);
				g_date_clear (&date, 1);

				/* get Julian dates for beginning and end of year */
				g_date_set_dmy (&date, 1, G_DATE_JANUARY, year);
				begin = g_date_get_julian (&date);
				g_date_clear (&date, 1);

				/* and the day before the beginning of the next year */
				g_date_set_dmy (&date, 1, G_DATE_JANUARY, year + 1);
				end =  g_date_get_julian (&date) - 1;
				
				switch (data->type)
				{
				case RHYTHMDB_QUERY_PROP_YEAR_EQUALS:
					restart_criteria = TRUE;
					data->type = RHYTHMDB_QUERY_SUBQUERY;
					data->subquery = rhythmdb_query_parse (db,
									       RHYTHMDB_QUERY_PROP_GREATER, data->propid, begin,
									       RHYTHMDB_QUERY_PROP_LESS, data->propid, end,
									       RHYTHMDB_QUERY_END);
					break;

				case RHYTHMDB_QUERY_PROP_YEAR_LESS:
					restart_criteria = TRUE;
					data->type = RHYTHMDB_QUERY_PROP_LESS;
					g_value_set_ulong (data->val, end);
					break;

				case RHYTHMDB_QUERY_PROP_YEAR_GREATER:
					restart_criteria = TRUE;
					data->type = RHYTHMDB_QUERY_PROP_GREATER;
					g_value_set_ulong (data->val, begin);
					break;

				default:
					break;
				}
				
				break;
			}
			
			default:
				break;
		}

		/* re-do this criteria, in case it needs further transformation */
		if (restart_criteria)
			i--;
	}
}

void
rhythmdb_query_append_prop_multiple (RhythmDB *db, GPtrArray *query, RhythmDBPropType propid, GList *items)
{
	GPtrArray *subquery;

	if (items == NULL)
		return;

	if (items->next == NULL) {
		rhythmdb_query_append (db,
				       query,
				       RHYTHMDB_QUERY_PROP_EQUALS,
				       propid,
				       items->data,
				       RHYTHMDB_QUERY_END);
		return;
	}

	subquery = g_ptr_array_new ();

	rhythmdb_query_append (db,
			       subquery,
			       RHYTHMDB_QUERY_PROP_EQUALS,
			       propid,
			       items->data,
			       RHYTHMDB_QUERY_END);
	items = items->next;
	while (items) {
		rhythmdb_query_append (db,
				       subquery,
				       RHYTHMDB_QUERY_DISJUNCTION,
				       RHYTHMDB_QUERY_PROP_EQUALS,
				       propid,
				       items->data,
				       RHYTHMDB_QUERY_END);
		items = items->next;
	}
	rhythmdb_query_append (db, query, RHYTHMDB_QUERY_SUBQUERY, subquery,
			       RHYTHMDB_QUERY_END);
}

gboolean
rhythmdb_query_is_time_relative (RhythmDB *db, GPtrArray *query)
{
	int i;
	if (query == NULL)
		return FALSE;

	for (i=0; i < query->len; i++) {
		RhythmDBQueryData *data = g_ptr_array_index (query, i);

		if (data->subquery) {
			if (rhythmdb_query_is_time_relative (db, data->subquery))
				return TRUE;
			else
				continue;
		}

		switch (data->type) {
		case RHYTHMDB_QUERY_PROP_CURRENT_TIME_WITHIN:
		case RHYTHMDB_QUERY_PROP_CURRENT_TIME_NOT_WITHIN:
			return TRUE;
		default:
			break;
		}
	}

	return FALSE;
}


GType
rhythmdb_query_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (type == 0)) {
		type = g_boxed_type_register_static ("RhythmDBQuery",
						     (GBoxedCopyFunc)rhythmdb_query_copy,
						     (GBoxedFreeFunc)rhythmdb_query_free);
	}

	return type;
}
