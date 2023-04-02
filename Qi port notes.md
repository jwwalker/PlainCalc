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

