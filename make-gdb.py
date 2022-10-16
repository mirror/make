"""GDB pretty-printer macros for GNU Make."""

import gdb  # pylint: disable=import-error
import gdb.printing  # pylint: disable=import-error


# Memoize types we commonly use
_TYPES = {}


def getType(tname):
    """Given a type name return a GDB type."""
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


def isNullptr(val):
    """Return True if the value is a null pointer."""
    return int(val.cast(getType('unsigned long long'))) == 0


class ShowArgv(gdb.Command):
    """Print a null-terminated array of strings.

    Argument:
        A char** where the last one is NULL (e.g., argv)
    """

    def __init__(self):
        """Create the showargv function."""
        gdb.Command.__init__(self, "showargv", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        """Show the argv."""
        args = gdb.string_to_argv(arg)
        if len(args) != 1:
            raise gdb.GdbError(self._usage)

        val = gdb.parse_and_eval(args[0])
        if val is None:
            raise gdb.GdbError('%s is not a valid expression' % (args[0]))

        strs = []
        while not isNullptr(val.dereference()):
            strs.append('"'+val.dereference().string()+'"')
            val += 1

        gdb.write("[%d] = [%s]\n" % (len(strs), ', '.join(strs)))
        gdb.flush()


ShowArgv()


class ShowNextList(gdb.Command):
    """Print a structure that has a "next" pointer.

    Argument:
        A pointer to a struct which contains a "next" member.
    """

    _usage = 'usage: showlist <listptr>'

    def __init__(self):
        """Create a "showlist" function."""
        gdb.Command.__init__(self, "showlist", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        """Show the elements in the provided list."""
        args = gdb.string_to_argv(arg)
        if len(args) != 1:
            raise gdb.GdbError(self._usage)

        val = gdb.parse_and_eval(args[0])
        if val is None:
            raise gdb.GdbError('%s is not a valid expression' % (args[0]))
        i = 0
        while not isNullptr(val):
            gdb.write("%s : %s\n" % (val, val.dereference()))
            gdb.flush()
            i += 1
            val = val['next']
        gdb.write("%s contains %d elements\n" % (args[0], i))
        gdb.flush()


ShowNextList()


class FileLocation(object):
    """Print a file location."""

    def __init__(self, val):
        """Create a FileLocation object."""
        self.val = val

    def to_string(self):
        """Convert a FileLocation to a string."""
        if int(self.val['filenm']):
            return "%s:%d" % (str(self.val['filenm']), self.val['lineno'])
        return 'NILF'


class StringListPrinter(object):
    """Print a stringlist."""

    def __init__(self, val):
        """Create a StringListPrinter object."""
        self.val = val

    def to_string(self):
        """Convert a HashTable into a string."""
        return "size=%d, capacity=%d" % (self.val['idx'], self.val['max'])

    def children(self):
        """Yield each string in the list."""
        i = 0
        elts = self.val['list']
        while i < self.val['idx']:
            nm = '[%d] ' % i
            yield (nm, elts.dereference())
            i += 1
            elts += 1

    def display_hint(self):
        """Show the display hint for the pretty-printer."""
        return 'array'


class VariablePrinter(object):
    """Print a struct variable."""

    def __init__(self, val):
        """Create a VariablePrinter object."""
        self.val = val

    def to_string(self):
        """Convert a VariablePrinter object into a string."""
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
    """Pretty-print a hash table."""

    DELITEM = None

    def __init__(self, val):
        """Create a HashTablePrinter object."""
        self.val = val

    def to_string(self):
        """Convert a HashTable into a string."""
        return "size=%d, capacity=%d, empty=%d, collisions=%d, rehashes=%d" % (
            self.val['ht_size'], self.val['ht_capacity'],
            self.val['ht_empty_slots'], self.val['ht_collisions'],
            self.val['ht_rehashes'])

    def children(self):
        """Yield each ID and value."""
        for (i, v) in self.iterator():
            nm = '[%d] ' % i
            yield (nm, i)
            yield (nm, v)

    def iterator(self):
        """Provide an iterator for HashTable."""
        if HashTablePrinter.DELITEM is None:
            HashTablePrinter.DELITEM = gdb.lookup_global_symbol('hash_deleted_item').value()
        lst = self.val['ht_vec']
        for i in range(0, self.val['ht_size']):
            v = lst[i]
            if int(v) != 0 and v != HashTablePrinter.DELITEM:
                yield (i, v)

    def display_hint(self):
        """Show the display hint for the pretty-printer."""
        return 'map'


class VariableSetPrinter(object):
    """Print a variable_set."""

    def __init__(self, val):
        """Create a variable_set pretty-printer."""
        self.tbl = HashTablePrinter(val['table'])

    def to_string(self):
        """Convert a variable_set to string."""
        return self.tbl.to_string()

    def children(self):
        """Iterate through variables and values."""
        for (i, v) in self.tbl.iterator():
            ptr = v.cast(getType('struct variable*'))
            nm = '[%d] ' % (i)
            yield (nm, ptr)
            yield (nm, str(ptr.dereference()))

    def display_hint(self):
        """Show the display hint for the pretty-printer."""
        return 'map'


class VariableSetListPrinter(object):
    """Print a variable_set_list."""

    GLOBALSET = None

    def __init__(self, val):
        """Create a variable_set_list pretty-printer."""
        self.val = val

    def to_string(self):
        """Convert a variable_set_list to string."""
        return str(self.val.address)

    def children(self):
        """Iterate through variables and values."""
        if VariableSetListPrinter.GLOBALSET is None:
            block = gdb.lookup_global_symbol('init_hash_global_variable_set').symtab.static_block()
            VariableSetListPrinter.GLOBALSET = gdb.lookup_symbol(
                'global_variable_set', block)[0].value().address
        ptr = self.val.address
        i = 0
        while not isNullptr(ptr):
            nm = '[%d] ' % (i)
            yield (nm, ptr['set'])
            if int(ptr['set']) == int(VariableSetListPrinter.GLOBALSET):
                yield (nm, "global_variable_set")
            else:
                yield (nm, str(ptr['set'].dereference()))
            i += 1
            ptr = ptr['next']

    def display_hint(self):
        """Show the display hint for the pretty-printer."""
        return 'map'


def build_pretty_printer():
    """Install all the pretty-printers."""
    pp = gdb.printing.RegexpCollectionPrettyPrinter("gnumake")
    pp.add_printer('floc', r'^floc$', FileLocation)
    pp.add_printer('stringlist', r'^stringlist$', StringListPrinter)
    pp.add_printer('variable', r'^variable$', VariablePrinter)
    pp.add_printer('hashtable', r'^hash_table$', HashTablePrinter)
    pp.add_printer('variableset', r'^variable_set$', VariableSetPrinter)
    pp.add_printer('variablesetlist', r'^variable_set_list$', VariableSetListPrinter)
    return pp


# Use replace=True so we can re-source this file
gdb.printing.register_pretty_printer(gdb.current_objfile(),
                                     build_pretty_printer(), replace=True)
