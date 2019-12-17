import collections
import logging
from . import CheckFileParser
import re

_logger = logging.getLogger(__name__)

class DirectiveException(Exception):
    def __init__(self, directive):
        self.directive = directive

    def __str__(self):
        return self.directive.getErrorMessage()

class Directive(object):
    def getName(self):
        return self.__class__.__name__

    def getErrorMessage(self):
        raise NotImplementedError()

    def match(self, subsetLines, subsetOffset, fileName):
        """
            Search through lines for match.
            What is returned is defined by implementations
        """
        raise NotImplementedError()

class RegexDirective(Directive):
    def __init__(self, pattern, sourceLocation):
        if not isinstance(pattern,str):
            raise Exception('Arg must be a string')

        if not isinstance(sourceLocation, CheckFileParser.FileLocation):
            raise Exception('sourceLocation must be a FileLocation')

        try:
            self.regex = re.compile(pattern)
        except Exception as e:
            raise CheckFileParser.ParsingException("Failed to parse regular expression at {location} : {reason}".format(
                                                   location=sourceLocation, reason=str(e)))

        self.sourceLocation = sourceLocation
        self.matchLocation = None
        self.failed = False

    def __str__(self):
        s = self.getName() + ' Directive ('
        s += "{file}:{line} Pattern: '{Regex}')".format(file=self.sourceLocation.fileName, line=self.sourceLocation.lineNumber, Regex=self.regex.pattern)
        return s


class LiteralDirective(Directive):
    def __init__(self, literal, sourceLocation):
        if not isinstance(literal,str):
            raise Exception('literal must be a string')

        if not isinstance(sourceLocation, CheckFileParser.FileLocation):
            raise Exception('sourceLocation must be a FileLocation')

        self.literal = literal
        if len(self.literal) == 0:
            raise CheckFileParser.ParsingException("Literal cannot be empty {location}".format(location=sourceLocation))

        self.sourceLocation = sourceLocation
        self.matchLocation = None
        self.failed = False

    def __str__(self):
        s = self.getName() + ' Directive ('
        s += "{file}:{line} Literal: '{literal}')".format(file=self.sourceLocation.fileName, line=self.sourceLocation.lineNumber, literal=self.literal)
        return s


class Check(RegexDirective):
    @staticmethod
    def directiveToken():
        return ':'

    def match(self, subsetLines, offsetOfSubset, fileName):
        """
            Search through lines for match.
            Raise an Exception if fail to match
            If match is succesful return the position the match was found
        """

        for (offset,l) in enumerate(subsetLines):
            m = self.regex.search(l)
            if m != None:
                truePosition = offset + offsetOfSubset
                _logger.debug('Found match on line {}'.format(str(truePosition+ 1)))
                _logger.debug('Line is {}'.format(l))
                self.matchLocation = CheckFileParser.FileLocation(fileName, truePosition +1) 
                return truePosition

        # No Match found
        self.failed = True
        raise DirectiveException(self)

    def getErrorMessage(self):
        return 'Could not find a match for {}'.format(str(self))

class CheckLiteral(LiteralDirective):
    @staticmethod
    def directiveToken():
        return '-L:'

    def match(self, subsetLines, offsetOfSubset, fileName):
        """
            Search through lines for match.
            Raise an Exception if fail to match
            If match is succesful return the position the match was found
        """

        for (offset,l) in enumerate(subsetLines):
            column = l.find(self.literal)
            if column != -1:
                truePosition = offset + offsetOfSubset
                _logger.debug('Found match on line {}, col {}'.format(str(truePosition+ 1), column))
                _logger.debug('Line is {}'.format(l))
                self.matchLocation = CheckFileParser.FileLocation(fileName, truePosition +1)
                return truePosition

        # No Match found
        self.failed = True
        raise DirectiveException(self)

    def getErrorMessage(self):
        return 'Could not find a match for {}'.format(str(self))

class CheckNext(RegexDirective):
    @staticmethod
    def directiveToken():
        return '-NEXT:'

    def __init__(self, pattern, sourceLocation):
        super(CheckNext,self).__init__(pattern, sourceLocation)
        self.expectedMatchLocation = None

    def match(self, line, positionOfLine, fileName):
        self.expectedMatchLocation = CheckFileParser.FileLocation(fileName, positionOfLine)

        m = self.regex.search(line)
        if m == None:
            self.failed = True
            raise DirectiveException(self)
        else:
            self.matchLocation = self.expectedMatchLocation
            _logger.debug('Found match for {pattern} at {location}'.format(pattern=self.regex.pattern, location=self.matchLocation))

    def getErrorMessage(self):
        return 'Could not find a match for {directive} expected at {location}'.format(directive=str(self), location=self.expectedMatchLocation)


class CheckNextLiteral(LiteralDirective):
    @staticmethod
    def directiveToken():
        return '-NEXT-L:'

    def __init__(self, literal, sourceLocation):
        super(CheckNextLiteral,self).__init__(literal, sourceLocation)
        self.expectedMatchLocation = None

    def match(self, line, positionOfLine, fileName):
        self.expectedMatchLocation = CheckFileParser.FileLocation(fileName, positionOfLine)

        column = line.find(self.literal)
        if column == -1:
            self.failed = True
            raise DirectiveException(self)
        else:
            self.matchLocation = self.expectedMatchLocation
            _logger.debug('Found match for {literal} at {location}'.format(literal=self.literal, location=self.matchLocation))

    def getErrorMessage(self):
        return 'Could not find a match for {directive} expected at {location}'.format(directive=str(self), location=self.expectedMatchLocation)

class CheckNot(RegexDirective):
    RegexLocationTuple = collections.namedtuple('RegexLocationTuple',['Regex','SourceLocation'])
    def __init__(self, pattern, sourceLocation):
        self.regex = []
        self.addPattern(pattern, sourceLocation)
        self.matchLocation = None

    @staticmethod
    def directiveToken():
        return '-NOT:'

    def addPattern(self, pattern, sourceLocation):
        if not isinstance(pattern,str):
            raise Exception('pattern must be a string')

        self.regex.append( self.RegexLocationTuple(Regex=re.compile(pattern), SourceLocation=sourceLocation) )

    def match(self, subsetLines, offsetOfSubset, fileName):
        """
            Search through lines for match.
            Raise an Exception if a match
        """
        for (offset,l) in enumerate(subsetLines):
            for t in self.regex:
                m = t.Regex.search(l)
                if m != None:
                    truePosition = offset + offsetOfSubset
                    _logger.debug('Found match on line {}'.format(str(truePosition+ 1)))
                    _logger.debug('Line is {}'.format(l))
                    self.failed = True
                    self.matchLocation = CheckFileParser.FileLocation(fileName, truePosition +1)
                    raise DirectiveException(self)

    def __str__(self):
        s = self.getName() + ' Directive ('
        for (index,p) in enumerate(self.regex):
            s += "{file}:{line} : Pattern: '{pattern}'".format(file=p.SourceLocation.fileName, line=p.SourceLocation.lineNumber, pattern=p.Regex.pattern)

            if index < (len(self.regex) -1):
                # Comma for all but last in list
                s+= ', '

            s += ")"

        return s

    def getErrorMessage(self):
        return 'Found a match for ' + str(self) + ' in {location}'.format(location=str(self.matchLocation))

class CheckNotLiteral(LiteralDirective):
    LiteralLocationTuple = collections.namedtuple('LiteralLocationTuple',['Literal','SourceLocation'])
    def __init__(self, literal, sourceLocation):
        self.literals = []
        self.addLiteral(literal, sourceLocation)
        self.matchLocation = None
        self.literal = None # We aren't using this

    @staticmethod
    def directiveToken():
        return '-NOT-L:'

    def addLiteral(self, literal, sourceLocation):
        if not isinstance(literal,str):
            raise Exception('literal must be a string')

        self.literals.append( self.LiteralLocationTuple(Literal=literal, SourceLocation=sourceLocation) )

    def match(self, subsetLines, offsetOfSubset, fileName):
        """
            Search through lines for match.
            Raise an Exception if a match
        """
        for (offset,l) in enumerate(subsetLines):
            for literal in self.literals:
                column = l.find(literal.Literal)
                if column != -1:
                    truePosition = offset + offsetOfSubset
                    _logger.debug('Found match on line {line}, column {col}'.format(line=str(truePosition+ 1), col=column))
                    _logger.debug('Line is {}'.format(l))
                    self.failed = True
                    self.matchLocation = CheckFileParser.FileLocation(fileName, truePosition +1)
                    raise DirectiveException(self)

    def __str__(self):
        s = self.getName() + ' Directive ('
        for (index,p) in enumerate(self.literals):
            s += "{file}:{line} : Literal: '{literal}'".format(file=p.SourceLocation.fileName, line=p.SourceLocation.lineNumber, literal=p.Literal)

            if index < (len(self.literals) -1):
                # Comma for all but last in list
                s+= ', '

            s += ")"

        return s

    def getErrorMessage(self):
        return 'Found a match for ' + str(self) + ' in {location}'.format(location=str(self.matchLocation))
