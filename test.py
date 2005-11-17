#!/usr/bin/env python
# options @ http://tidy.sourceforge.net/docs/quickref.html
import tinytidy

options = {
    'indent': 1,
    'markup': 1,
    'add-xml-decl': 1,
    'output-xhtml': 1
    }

print tinytidy.parseString('<title>Foo</title><p>Foo!', options)
