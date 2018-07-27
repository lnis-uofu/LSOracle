#!/usr/bin/env python
# vim: set sw=2 ts=4 softabstop=4 expandtab
"""
Output checker inspired by LLVM's FileCheck.

See https://github.com/stp/OutputCheck for documentation.
"""
import os
import argparse
import re
import logging
from . import CheckFileParser
from . import Directives
from . import FileChecker
from . import CommentPrefixes

_logger = logging.getLogger(__name__)

def enum(*sequential, **named):
    enums = dict(zip(sequential, range(len(sequential))), **named)
    reverse = dict((value, key) for key, value in enums.items())
    enums['reverse_mapping'] = reverse
    return type('Enum', (), enums)

ExitCode = enum('SUCCESS',
                'PARSE_ERROR',
                'DIRECTIVE_FAIL',
                'FILE_WITHOUT_SUFFIX',
                'UNSUPPORTED_FILE_TYPE',
                'USER_EXIT'
               )



def main(args):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('check_file', type=argparse.FileType('rU'), help='File containing check commands')
    parser.add_argument('--file-to-check=', type=argparse.FileType('r'), default='-', help='File to check (default %(default)s)')
    parser.add_argument('--check-prefix=', default='CHECK', help='Prefix to use from check_file')
    parser.add_argument("-l","--log-level",type=str, default="INFO", choices=['debug','info','warning','error'])
    parser.add_argument('--comment=',type=str, default="", help='Force one line comment value. Default guess from file extension of check_file')
    parser.add_argument('-d','--dump-file-to-check', default=False, action='store_true', help='Print file-to-check contents')
    parser.add_argument('--disable-substitutions', default=False, action='store_true', help='Disable ${LINE} style substitutions')
    args = parser.parse_args(args[1:])

    logLevel = getattr(logging,args.log_level.upper(),None)
    if logLevel == logging.DEBUG:
        logFormat = '%(levelname)s:%(filename)s:%(lineno)d %(funcName)s()  : %(message)s'
    else:
        logFormat = '%(levelname)s: %(message)s'


    logging.basicConfig(level=logLevel, format=logFormat)

    checkFile = args.check_file
    fileToCheck = getattr(args,'file_to_check=')
    checkDirectivePrefix = getattr(args,'check_prefix=')
    requestedLineCommentPrefix = getattr(args,'comment=')
    lineCommentPrefix = None
    FC = None

    def printInput():
        if args.dump_file_to_check and FC.getInput():
            _logger.info('File to check contained:')
            for l in FC.getInput():
                print(l.rstrip("\n"))

    try:
        if len(requestedLineCommentPrefix) == 0:
            lineCommentPrefix = CommentPrefixes.getLineCommentPrefix(checkFile.name)
        else:
            lineCommentPrefix = requestedLineCommentPrefix
            _logger.info('Assuming single line comment prefix is {prefix}'.format(prefix=lineCommentPrefix))

        _logger.debug("Line comment prefix is '{}'".format(lineCommentPrefix))
        checkFileParser = CheckFileParser.CheckFileParser(checkDirectivePrefix, lineCommentPrefix)
        FC = FileChecker.FileChecker(checkFileParser.parse(checkFile, not args.disable_substitutions))
        FC.check(fileToCheck)
        printInput()
    except CheckFileParser.ParsingException as e:
        _logger.error(e)
        return ExitCode.PARSE_ERROR
    except Directives.DirectiveException as e:
        _logger.error(e)
        printInput()
        return ExitCode.DIRECTIVE_FAIL
    except CommentPrefixes.FileWithoutSuffixException as e:
        _logger.error("Check file '{file}' is missing a file extension".format(file=checkFile.name))
        _logger.info('If you know what symbols are used for one line comments then use the --comment= flag')
        return ExitCode.FILE_WITHOUT_SUFFIX
    except CommentPrefixes.UnSupportedFileTypeException as e:
        _logger.error(e)
        _logger.info('If you know what symbols are used for one line comments then use the --comment= flag')
        return ExitCode.UNSUPPORTED_FILE_TYPE
    except KeyboardInterrupt as e:
        return ExitCode.USER_EXIT

    return ExitCode.SUCCESS

def scriptEntryPoint():
    """
    This provides an entry point for disutils
    """
    import sys
    return main(sys.argv)
