# Qi Porting Notes

## Observations

* In declaring a rule, you can omit the signature parameter, and just give the iterator
and the skipper.

* It appears that in the declaration of a grammar, the template parameters of `qi::grammar`
can omit the signature parameter.

* To get the raw matching text of a parser, I can write `qi::raw[parser]`
and attach a semantic action with a method
`void operator()( boost::iterator_range<const char*>& s ) const`.

* In classic Spirit, the prefix operator `!` means that the following parser is optional
(match 0 or 1 times), while in Qi, prefix operator `!` means not (do not match), while the
prefix operator `-` means optional.


## Why Does the calculator task end with status 4?

Seems like this had something to do with sandbox and entitlements.

"To enable sandbox inheritance, a child target must use exactly two App Sandbox
entitlement keys: com.apple.security.app-sandbox and com.apple.security.inherit. If you
specify any other App Sandbox entitlement, the system aborts the child process. "

Entitlements of main app:

```
{
    "com.apple.security.app-sandbox" = 1;
    "com.apple.security.files.user-selected.read-write" = 1;
    "com.apple.security.get-task-allow" = 1;
}
```

The tool apparently has entitlements

```
{
    "com.apple.security.app-sandbox" = 1;
    "com.apple.security.inherit" = 1;
    "com.apple.security.temporary-exception.files.absolute-path.read-only" =     (
        "/"
    );
    "com.apple.security.temporary-exception.mach-lookup.global-name" =     (
        "com.apple.testmanagerd",
        "com.apple.dt.testmanagerd.runner",
        "com.apple.coresymbolicationd"
    );
}
```

See [Embedding a command-line tool in a sandboxed app](
https://developer.apple.com/documentation/xcode/embedding-a-helper-tool-in-a-sandboxed-app)


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
