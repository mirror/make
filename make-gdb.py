import re

import gdb #pylint: disable=import-error
import gdb.printing #pylint: disable=import-error

# Memoize types we commonly use
_TYPES = {}
def getType(tname):
    global _TYPES
    if tname not in _TYPES:
        tn = tname.rstrip('*')
        if tn not in _TYPES:
            _TYPES[tn] = gdb.lookup_type(tn)
        while tn != tname:
            # Want a pointer type
            t = tn
            tn += '*'
            _TYPES[tn] = _TYPES[t].pointer()
    return _TYPES[tname]

class ShowArgv(gdb.Function):
    """Return the pretty-print of a null-terminated array of strings

    Argument:
        A char** where the last one is NULL (e.g., argv)
    """

    def __init__(self):
        gdb.Function.__init__(self, "showargv")

    def invoke(self, argv):
        str = '['
        i = 0
        while argv[i] != 0:
            if i > 0:
                str += ', '
            str += argv[i].string()
            i += 1
        str += ']'
        return str

ShowArgv()


class FileLocation(object):
    """Print a file location"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        if long(self.val['filenm']):
            return "%s:%d" % (str(self.val['filenm']), self.val['lineno'])
        return ''

class VariablePrinter(object):
    """Print a struct variable"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        if self.val['append']:
            a = '+='
        elif self.val['conditional']:
            a = '?='
        else:
            a = '='
        flags = []
        s = str(self.val['flavor'])
        if s != 'f_bogus':
            flags.append(s)
        s = str(self.val['origin'])
        if s != 'o_default':
            flags.append(s)
        s = str(self.val['export'])
        if s != 'v_default':
            flags.append(s)
        return '%s[%s]: "%s" %s "%s"' % (
            self.val['fileinfo'], ','.join(flags),
            self.val['name'].string(), a, self.val['value'].string())

class HashTablePrinter(object):
    """Manage a hash table."""

    DELITEM = None

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "size=%d, capacity=%d, empty=%d, collisions=%d, rehashes=%d" % (
            self.val['ht_size'], self.val['ht_capacity'],
            self.val['ht_empty_slots'], self.val['ht_collisions'],
            self.val['ht_rehashes'])

    def children(self):
        for (i, v) in self.iterator():
            nm = '[%d] ' % i
            yield (nm, i)
            yield (nm, v)

    def iterator(self):
        if HashTablePrinter.DELITEM is None:
            HashTablePrinter.DELITEM = gdb.lookup_global_symbol('hash_deleted_item').value()
        lst = self.val['ht_vec']
        for i in xrange(0, self.val['ht_size']):
            v = lst[i]
            if long(v) != 0 and v != HashTablePrinter.DELITEM:
                yield (i, v)

    def display_hint(self):
        return 'map'

class VariableSetPrinter(object):
    """Print a variable_set"""

    def __init__(self, val):
        self.tbl = HashTablePrinter(val['table'])

    def to_string(self):
        return self.tbl.to_string()

    def children(self):
        for (i, v) in self.tbl.iterator():
            ptr = v.cast(getType('struct variable*'))
            nm = '[%d] ' % (i)
            yield (nm, ptr)
            yield (nm, str(ptr.dereference()))

    def display_hint(self):
        return 'map'

class VariableSetListPrinter(object):
    """Print a variable_set_list"""

    GLOBALSET = None

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return str(self.val.address)

    def children(self):
        if VariableSetListPrinter.GLOBALSET is None:
            block = gdb.lookup_global_symbol('init_hash_global_variable_set').symtab.static_block()
            VariableSetListPrinter.GLOBALSET = gdb.lookup_symbol('global_variable_set', block)[0].value().address
        ptr = self.val.address
        i = 0
        while long(ptr) != 0:
            nm = '[%d] ' % (i)
            yield (nm, ptr['set'])
            if long(ptr['set']) == long(VariableSetListPrinter.GLOBALSET):
                yield (nm, "global_variable_set")
            else:
                yield (nm, str(ptr['set'].dereference()))
            i += 1
            ptr = ptr['next']

    def display_hint(self):
        return 'map'

def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("gnumake")
    pp.add_printer('floc', r'^floc$', FileLocation)
    pp.add_printer('variable', r'^variable$', VariablePrinter)
    pp.add_printer('hashtable', r'^hash_table$', HashTablePrinter)
    pp.add_printer('variableset', r'^variable_set$', VariableSetPrinter)
    pp.add_printer('variablesetlist', r'^variable_set_list$', VariableSetListPrinter)
    return pp

# Use replace=True so we can re-source this file
gdb.printing.register_pretty_printer(gdb.current_objfile(),
                                     build_pretty_printer(), replace=True)
