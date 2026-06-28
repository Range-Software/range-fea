# Fix sliver elements

Detect and automatically fix sliver elements.

An element is declared a sliver by its longest to shortest edge ratio.

```
  |\
  | \
  |  \ c
a |   \
  |    \
  |_____\
     b
  where: edge ratio = c / b
```
