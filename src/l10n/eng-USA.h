////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2010 - 2015, Göteborg Bit Factory.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// This file contains all the strings that should be localized.  If a string is
// *not* in this file, then either:
//   (a) it should not be localized, or
//   (b) you have found a bug - please report it
//
// Strings that should be localized:
//   - text output that the user sees
//
// Strings that should NOT be localized:
//   - .taskrc configuration variable names
//   - command names
//   - extension function names
//   - certain literals associated with parsing
//   - debug strings
//   - attribute names
//   - modifier names
//   - logical operators (and, or, xor)
//
// Rules:
//   - Localized strings should not in general  contain leading or trailing
//     white space, including \n, thus allowing the code to compose strings.
//   - Retain the tense of the original string.
//   - Retain the same degree of verbosity of the original string.
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// Translators:
//   1. Copy this file (eng-USA.h) to a new file with the target locale as the
//      file name.  Using German as an example, do this:
//
//        cp eng-USA.h deu-DEU.h
//
//   2. Modify all the strings below.
//        i.e. change "Unknown error." to "Unbekannter Fehler.".
//
//   3. Add your new translation to the task.git/src/i18n.h file, if necessary,
//      by inserting:
//
//        #elif PACKAGE_LANGUAGE == LANGUAGE_DE_DE
//        #include <deu-DEU.h>
//
//   4. Add your new language to task.git/CMakeLists.txt, making sure that
//      number is unique:
//
//        set (LANGUAGE_DE_DE 4)
//
//   5. Add your new language to task.git/cmake.h.in:
//
//        #define LANGUAGE_DE_DE ${LANGUAGE_DE_DE}                                        
//
//   6. Build your localized Taskwarrior with these commands:
//
//      cd task.git
//      cmake -D LANGUAGE=3 .
//      make
//
//   7. Submit your translation to support@taskwarrior.org, where it will be
//      shared with others.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_STRINGS
#define INCLUDED_STRINGS

// Note that for English, the following two lines are not displayed.  For all
// other localizations, these lines will appear in the output of the 'version'
// and 'diagnostics' commands.
//
// DO NOT include a copyright in the translation.
//
#define STRING_LOCALIZATION_DESC     "English localization"
#define STRING_LOCALIZATION_AUTHOR   "Translated into English by A. Person."

#define STRING_TASK_NO_FF1           "Taskwarrior no longer supports file format 1, originally used between 27 November 2006 and 31 December 2007."
#define STRING_TASK_NO_FF2           "Taskwarrior no longer supports file format 2, originally used between 1 January 2008 and 12 April 2009."
#define STRING_TASK_NO_ENTRY         "Annotation is missing an entry date: {1}"
#define STRING_TASK_NO_DESC          "Annotation is missing a description: {1}"

#define STRING_TASK_PARSE_ANNO_BRACK "Missing annotation brackets."
#define STRING_TASK_PARSE_ATT_BRACK  "Missing attribute brackets."
#define STRING_TASK_PARSE_TAG_BRACK  "Missing tag brackets."
#define STRING_TASK_PARSE_TOO_SHORT  "Line too short."
#define STRING_TASK_PARSE_UNREC_FF   "Unrecognized taskwarrior file format."

#define STRING_RECORD_EMPTY          "Empty record in input."
#define STRING_RECORD_JUNK_AT_EOL    "Unrecognized characters at end of line."
#define STRING_RECORD_NOT_FF4        "Record not recognized as format 4."
#define STRING_RECORD_LINE           " at line {1}"

#define STRING_TASK_VALID_DESC       "A task must have a description."
#define STRING_TASK_VALID_BLANK      "Cannot add a task that is blank."
#define STRING_TASK_VALID_REC_DUE    "A recurring task must also have a 'due' date."
#define STRING_TASK_VALID_RECUR      "The recurrence value '{1}' is not valid."

#define STRING_JSON_VALIDATE         "You must specify either a JSON string or a JSON file."
#define STRING_JSON_SYNTAX_OK        "JSON syntax ok."

#define STRING_TASKD_DATA            "ERROR: The '--data' option requireѕ an argument."

#define STRING_CONFIG_NO_PATH        "ERROR: The '--data' path does not exist."
#define STRING_CONFIG_OVERWRITE      "Are you sure you want to change the value of '{1}' from '{2}' to '{3}'?"
#define STRING_CONFIG_ADD            "Are you sure you want to add '{1}' with a value of '{2}'?"
#define STRING_CONFIG_MODIFIED       "Config file {1} modified."
#define STRING_CONFIG_NO_CHANGE      "No changes made."
#define STRING_CONFIG_REMOVE         "Are you sure you want to remove '{1}'?"
#define STRING_CONFIG_NOT_FOUND      "ERROR: No entry named '{1}' found."
#define STRING_CONFIG_SOURCE         "Configuration read from {1}"
#define STRING_CONFIG_READ_ONLY      "Configuration file is read-only, no changes possible."

#define STRING_INIT_DATA_REQUIRED    "ERROR: The '--data' option is required."
#define STRING_INIT_PATH_MISSING     "ERROR: The '--data' path does not exist."
#define STRING_INIT_PATH_NOT_DIR     "ERROR: The '--data' path is not a directory."
#define STRING_INIT_PATH_NOT_READ    "ERROR: The '--data' directory is not readable."
#define STRING_INIT_PATH_NOT_WRITE   "ERROR: The '--data' directory is not writable."
#define STRING_INIT_PATH_NOT_EXE     "ERROR: The '--data' directory is not executable."
#define STRING_INIT_SERVER           "You must specify the 'server' variable before attempting a server start, for example:"
#define STRING_INIT_COULD_NOT_CREATE "ERROR: Could not create '{1}'."
#define STRING_INIT_CREATED          "Created {1}"

#define STRING_CLIENT_USAGE          "ERROR: Usage:  taskd client [options] <host:post> <file> [<file> ...]"
#define STRING_CLIENT_DISABLED       "ERROR: Client interface feature not enabled."

#define STRING_TASKD_BAD_COMMAND     "ERROR: Did not recognize command '{1}'."

#define STRING_ERROR_UNKNOWN         "Unknown error"
#define STRING_SERVER_DOWN           "ERROR: Taskserver not responding."

#endif

