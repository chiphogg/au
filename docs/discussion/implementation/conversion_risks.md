# Conversion Risks

Units libraries implement _unit conversions_ as sequences of operations: multiplying and dividing by
different numbers, casting to different types, and so on.

Beware: in any units library, unit conversions can sometimes produce grossly incorrect results!
There are two main categories of risk.  For any given operation in the conversion,

- **Overflow** means that the result is _outside the range of values_ for the type.
- **Truncation** means that the result is _in-between representable values_ for the type.

Small types make it easy to illustrate these ideas.  Consider `int8_t`, whose max value is `127`,
and which can only represent integers.  If we convert 21 feet to inches, it will _overflow_, because
the resulting number of inches (252) exceeds that max value.  Conversely, if we convert 21 inches to
feet, it will _truncate_, because the resulting number of feet (1.75) is not an integer, and we will
store the value `1` instead.

This doc discusses Au's philosophical approach to overflow and truncation, which together we call
the "conversion risks".

## Formulating the problem

- old rep, new rep, conversion factor
- translate into sequence of operations (see applying magnitude page)

## Mitigating each risk

### Overflow

### Truncation








# What I didn't tell you yet

- API ideas for skipping one risk or the other
- Just because you allow the risk, doesn't mean we can perform the computation!  Example: irrational
  numbers in integral types.
