# Qi Porting Notes

## Observations

* In declaring a rule, you can omit the signature parameter, and just give the iterator
and the skipper.

* It appears that in the declaration of a grammar, the template parameters of `qi::grammar`
can omit the signature parameter.

* To get the raw matching text of an `expressionNA` rule, I can write `qi::raw[expressionNA]`
and attach a semantic action with a method
`void operator()( boost::iterator_range<const char*>& s ) const`.

## push_back_a is gone

## assign operator is gone

Usage:

				assignment =
					identifier[ assign(mState.mIdentifier) ]
					>> '=' >> expression;

				funcdef = identifier[ assign(mState.mFuncName) ]
					>> '(' >> identifier[ push_back_a(mState.mParamStack) ]
					>>	*(
							',' >> identifier[ push_back_a(mState.mParamStack) ]
						)
					>> ')' >> '='
					>> lexeme[*standard::char_][ assign(mState.mFuncDef) ];

					|	( "if(" >> expression >> ','
							>> expressionNA[ assign(mState.mIf1) ] >> ','
							>> expressionNA[ assign(mState.mIf2) ] >> ')'
						)[ DoIf( mState ) ]

What is being assigned to is always a `std::string` member of `SCalcState`.  The parsers
that it is attached to are `identifier`, `lexeme`, and `expressionNA`.

The parser `lexeme[*standard::char_]` should have an attribute that is `vector<char>&`.

What about `lexeme[ standard::alpha >> *(standard::alnum | '_') ]`, the main part of
`identifier`?  It appears that the type is
`boost::fusion::vector<char, std::vector< boost::optional<char> > >`.
The attribute type of `standard::alnum | '_'` is `boost::optional<char>`.
If I change that `'_'` to `qi::char_("_")`, then the attribute type is just `char`.
So, the attribute type of `qi::lexeme[ standard::alpha >> *(standard::alnum | qi::char_("_")) ]`
is `boost::fusion::vector<char, std::vector<char>>`.


## Problematic Actions

### DoUnaryFunc

Usage:

	|	(lexeme[mState.mFixed.mUnaryFuncs >> '('] >> expression
			>> ')')[ DoUnaryFunc(mState) ]

The only part of the parsed text that the action actually wants to see is the initial
lexeme, so maybe it can be done with two actions, one to grab the function name and one to
evaluate it.

### DoBinaryFunc

### DoDefinedFunc

### DebugAction
