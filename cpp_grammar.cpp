from pyparsing import *

class CPPGrammar:
    def __init__(self):
        identifier = Word(alphas + "_", alphanums + "_")
        integerLiteral = Word(nums)
        floatingPointLiteral = Combine(Word(nums) + '.' + Word(nums))
        characterLiteral = QuotedString("'", escChar='\\')
        stringLiteral = QuotedString('"', escChar='\\')

        literal = integerLiteral | floatingPointLiteral | characterLiteral | stringLiteral

        primaryExpression = identifier | literal | nestedExpr(opener='(', closer=')')

        multiplicativeExpression = operatorPrecedence(primaryExpression, [
            (oneOf('* /'), 2, opAssoc.LEFT),
        ])

        additiveExpression = operatorPrecedence(multiplicativeExpression, [
            (oneOf('+ -'), 2, opAssoc.LEFT),
        ])

        expression = additiveExpression

        type_ = oneOf('int float double char bool')

        ifStatement = Group('if' + '(' + expression + ')' + '{' + expression + '}' + Optional('else' + '{' + expression + '}'))

        assignment = identifier + '=' + expression + ';'

        variableDeclaration = type_ + identifier + '=' + expression + ';'

        statement = variableDeclaration | assignment | ifStatement

        self.compilationUnit = ZeroOrMore(statement)
