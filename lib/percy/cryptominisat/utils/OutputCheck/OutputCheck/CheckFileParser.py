import collections
import logging
import os
import pprint
import re
from .Utils import isA
# Cannot import Directives here due to circular dependencies

_logger = logging.getLogger(__name__)

class FileLocation(object):
    def __init__(self, fileName, lineNumber):
        self.fileName = fileName
        self.lineNumber = lineNumber

    def __str__(self):
        return self.fileName + ':' + str(self.lineNumber)

class ParsingException(Exception):
    def __init__(self, msg):   
        super(self.__class__,self).__init__(msg)

    def __str__(self):
        return str(self.__class__.__name__) + ": " + self.args[0]

class CheckFileParser:
    def  __init__(self, checkPrefix, lineCommentPrefix):
        self.checkPrefix = checkPrefix
        self.lineCommentPrefix = lineCommentPrefix

        # Find directives
        import inspect
        directivePrefix = r'^\s*' + self.lineCommentPrefix + r'\s*' + self.checkPrefix
        patternRegex = r'(.+)$'
        directive = collections.namedtuple('Directive',['Regex','Class'])
        
        directives = []

        # Try to find Directives dynamically
        from . import Directives
        for (name,object) in Directives.__dict__.items():
            if not inspect.isclass(object):
                continue

            if getattr(object,'directiveToken',None) != None:
                # Found a match
                directives.append( directive( Regex=re.compile(directivePrefix + 
                                                               object.directiveToken() + 
                                                               '\s*' + # Ignore all whitespace after directive
                                                               patternRegex),
                                              Class=object
                                            ) 
                                 )

        self.directives = directives
        _logger.debug('Found directives:\n{}'.format(pprint.pformat(self.directives)))

    def parse(self, checkFile, doSubstitutions=True):
        from . import Directives
        assert isinstance(doSubstitutions, bool)
        _logger.debug('Substitutions enabled:{}'.format(doSubstitutions))
        directiveObjects = []

        lineNumber=1
        location = None
        lines = checkFile.readlines()
        for line in lines:
            for d in self.directives:
                m = d.Regex.match(line)
                if m != None:
                    if doSubstitutions:
                        checkPattern = self._substituteCheckPattern(m.group(1), lineNumber, len(lines), checkFile.name)
                    else:
                        checkPattern = m.group(1)
                    location = FileLocation(checkFile.name, lineNumber)

                    if ( len(directiveObjects) > 0 and
                         isinstance(directiveObjects[-1], Directives.CheckNot) and
                         d.Class == Directives.CheckNot
                       ):
                        # Do not allow consecutive CHECK-NOT directives, just add
                        # pattern to previous CHECK-NOT
                        directiveObjects[-1].addPattern(checkPattern, location)
                        _logger.debug('{file}:{line} : Added pattern {pattern} to directive\n{directive}'.format(file=checkFile.name,
                                                                                                                  line=lineNumber,
                                                                                                                  pattern=checkPattern,
                                                                                                                  directive=directiveObjects[-1]))
                    elif ( len(directiveObjects) > 0 and
                         isinstance(directiveObjects[-1], Directives.CheckNotLiteral) and
                         d.Class == Directives.CheckNotLiteral
                       ):
                        # Do not allow consecutive CHECK-NOT-L directives, just add
                        # literal to previous CHECK-NOT-L
                        directiveObjects[-1].addLiteral(checkPattern, location)
                        _logger.debug('{file}:{line} : Added Literal {literal} to directive\n{directive}'.format(file=checkFile.name,
                                                                                                                 line=lineNumber,
                                                                                                                 literal=checkPattern,
                                                                                                                 directive=directiveObjects[-1]))
                    else:
                        # Create new Directive Object with a pattern (as string)
                        directiveObjects.append( d.Class(checkPattern, location) )
                        _logger.debug('{file}:{line} : Creating directive\n{directive}'.format(file=checkFile.name, line=lineNumber, directive=directiveObjects[-1]))

            lineNumber += 1

        self._validateDirectives(directiveObjects, checkFile.name)
        return directiveObjects

    def _validateDirectives(self, directiveList, checkFileName):

        if len(directiveList) == 0:
            raise ParsingException("'{file}' does not contain any CHECK directives".format(file=checkFileName))

        from . import Directives
        """
            We should enforce for every CHECK-NOT and CHECK-NOT-L directive that the next directive (if it exists) is
            a CHECK or CHECK-L directive
        """
        last = len(directiveList) -1
        supportedDirectives = [ Directives.Check, Directives.CheckLiteral ]
        for (index,directive) in enumerate(directiveList):
            if isA(directive, [Directives.CheckNot, Directives.CheckNotLiteral]):
                if index < last:
                    after = directiveList[index +1]
                    if not isA(after, supportedDirectives):
                        requiredTypes = " or ".join( [ "CHECK{suffix}".format(suffix=d.directiveToken()) for d in supportedDirectives])
                        raise ParsingException("{directive} must have a {requiredTypes} directive after it instead of a {bad}".format(
                                                  directive=directive,
                                                  requiredTypes=requiredTypes,
                                                  check=Directives.Check.directiveToken(),
                                                  bad=after)
                                              )

    def _substituteCheckPattern(self, inputString, lineNumber, lastLineNumber, checkFileName):
        """
        Do various ${} substitutions
        """
        assert isinstance(inputString, str)
        assert isinstance(lineNumber, int)
        assert isinstance(lastLineNumber, int)
        assert isinstance(checkFileName, str)

        """
        Do ${LINE}, ${LINE:+N}, and ${LINE:-N} substitutions.
        To escape prepend with slash
        """
        sPattern = r'\$\{LINE(\:(?P<sign>\+|-)(?P<offset>\d+))?\}'
        matcher = re.compile(sPattern)
        result = ""
        loop = True
        start = 0
        end = len(inputString) # Not inclusive
        while loop:
            m = matcher.search(inputString, start, end)
            if not m:
                # No match so copy verbatim
                _logger.debug('Result is currently "{}"'.format(result))
                result += inputString[start:end]
                break # And we're done :)
            else:
                prevIndex = max(0, m.start() -1)
                _logger.debug('Previous character before match is at index {index} "{char}"'.format(index=prevIndex, char=inputString[prevIndex]))
                if inputString[prevIndex] == "\\":
                    # User asked to escape
                    _logger.debug('Substitution is escaped')
                    _logger.debug('Result is currently "{}"'.format(result))
                    result += inputString[start:prevIndex] # Copy before escaping character
                    _logger.debug('Result is currently "{}"'.format(result))
                    result += inputString[(prevIndex+1):m.end()] # Copy the ${LINE..} verbatim
                    start = min(m.end(), end)
                    _logger.debug('Result is currently "{}"'.format(result))
                    _logger.debug('Next search is {start}:{end} = "{ss}"'.format(start=start, end=end, ss=inputString[start:end]))
                else:
                    _logger.debug('Result is currently "{}"'.format(result))
                    _logger.debug('Doing subsitution. Found at {begin}:{end} = {ss}'.format(begin=m.start(),end=m.end(), ss=inputString[m.start():m.end()]))
                    result += inputString[start:m.start()] # Copy before substitution starts

                    if m.groupdict()['sign'] == None:
                        # No offset just substitute line number
                        _logger.debug('No offset')
                        result += str(lineNumber)
                    else:
                        offset = 1 if m.groupdict()['sign'] == '+' else -1
                        offset *= int(m.groupdict()['offset'])
                        _logger.debug('Offset is {}'.format(offset))

                        requestedLineNumber = lineNumber + offset
                        _logger.debug('Request line number to print is  {}'.format(requestedLineNumber))

                        if requestedLineNumber <= 0:
                            raise ParsingException('{file}:{line}:{col} offset gives line number < 1'.format(file=checkFileName, line=lineNumber, col=m.start()))
                        elif requestedLineNumber > lastLineNumber:
                            raise ParsingException('{file}:{line}:{col} offset gives line number past the end of file'.format(file=checkFileName, line=lineNumber, col=m.start()))

                        result += str(requestedLineNumber)

                    start = min(m.end(),end)
                    _logger.debug('Next search is {start}:{end} = "{ss}"'.format(start=start, end=end, ss=inputString[start:end]))

        """
        Do simple ${...} substitutions
        """

        # Do ${CHECKFILE_NAME} substitution
        result = self._simpleSubstitution("CHECKFILE_NAME", os.path.basename(checkFileName), result)

        # Do ${CHECKFILE_ABS_PATH} substitution
        result = self._simpleSubstitution("CHECKFILE_ABS_PATH", os.path.abspath(checkFileName), result)

        assert len(result) != 0
        return result

    def _simpleSubstitution(self, name, replacement, inputString):
        assert isinstance(name, str)
        assert isinstance(replacement, str)
        assert isinstance(inputString, str)
        stringToMatch = '${' + name + '}'
        result = ""
        start = 0
        end = len(inputString)

        while True:
            pos = inputString.find(stringToMatch, start, end)
            if pos == -1:
                # No match found
                result += inputString[start:end]
                break
            else:
                # Check for escaping slash
                posBefore=max(0, pos -1)
                if inputString[posBefore] == "\\":
                    # Escaped
                    result += inputString[start:posBefore] # Copy before \${...}
                    result += inputString[(posBefore+1): (pos + len(stringToMatch))] # Copy ${...}, note skipping slash
                    start = min(pos + len(stringToMatch), end)
                else:
                    # Do substitution
                    result += inputString[start:pos] # Copy before ${...}
                    result += replacement
                    start = min(pos + len(stringToMatch), end)

        assert len(result) != 0
        return result
