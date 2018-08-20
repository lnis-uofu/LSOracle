import logging
from . import Directives
from .Utils import isA

_logger = logging.getLogger(__name__)

class FileChecker(object):
    def __init__(self, directiveList):
        self.directives = directiveList
        self.inputLines = None

    def check(self, fileObject):
        lines = fileObject.readlines()

        # Filter out line ending characters
        lines = list(map( lambda line: line.strip('\r\n'), lines))
        self.inputLines = lines

        lineNum=0 # Starts from 0
        end = len(lines)

        dIndex = 0
        while dIndex < len(self.directives):
            checker = self.directives[dIndex]

            if isA(checker, [ Directives.Check, Directives.CheckLiteral]):
                offset = checker.match(lines[lineNum:end], lineNum, fileObject.name)
                lineNum = min(offset +1, end) # Start next search from next line
            elif isA(checker, [Directives.CheckNot, Directives.CheckNotLiteral]):
                # We need to know the region that CheckNot/CheckNotLiteral should search.
                # We assume that either there are no other Directives after this one
                # or the next directive is a Check Directive

                if dIndex + 1 == len(self.directives):
                    # This is the last directive
                    checker.match(lines[lineNum:end], lineNum, fileObject.name)
                elif isA( self.directives[dIndex +1], [Directives.Check, Directives.CheckLiteral] ):
                    # We need to invoke the subsequent Check directive to find
                    # the region we should use for CheckNot
                    endRegion = self.directives[dIndex +1].match(lines[lineNum:end], lineNum, fileObject.name)

                    # Now invoke the CheckNot directive
                    checker.match(lines[lineNum:endRegion], lineNum, fileObject.name)

                    # Assuming everything went okay update lineNum to the region just pass what the
                    # Check directive found
                    lineNum = min(endRegion+1, end)

                    dIndex +=2
                    continue
                else:
                    raise Exception('Internal Error')
            elif isA(checker, [Directives.CheckNext, Directives.CheckNextLiteral]):
                checker.match(lines[lineNum], lineNum + 1, fileObject.name)
                lineNum = min(lineNum +1, end)
            else:
                raise Exception('Directive {} unsupported'.format(checker))

            dIndex += 1

    def getInput(self):
        return self.inputLines
