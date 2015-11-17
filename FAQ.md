**What is CSVTest?**

CSVTest is a data generator that produces CSV (comma-separated values) data suitable for use in relational database and other applications.

**Why would I want to generate data?**

Well, if you have to ask, maybe you don't. But generated data is useful in many different circumstances - for example:

  * Creating realistic demonstrations of your application
  * Test-driven design
  * Performance testing
  * Database sizing exercises

**What kind of data can CSVTest generate?**

Many different kinds. Out of the box it supports:

  * Random and non-random integers, reals, dates and times
  * Random selection from CSV data files
  * Random strings
  * Foreign key and many-to-many relationships
  * Numeric, date and time ranges

All of the above can be composed to create new data types.

**What output formats does CSVTest support?**

Only CSV - hence the name. You can use its sister program CSVfix to produce different formats, such as SQL INSERT statements.

**What license does CSVTest use?**

The MIT License. Basically, you can do anything you want with the software and the code.

**How do I use it?**

You write a script using XML, and run it through the CSVTest executable. Here's a script called dice.xml that simulates rolling a 6-sided dice.

```
<csvt>
    <gen>
        <rand_int begin="1" end="7">
        </rand_int>
    </gen>
</csvt>
```

You run the script like this:

```
   csvtest dice.xml
```
which might produce the exciting output:

```
"3"
```

**That is deeply unimpressive - I can do that in Perl, Python etc.**

True enough. Most simple uses of CSVTest can be done as easily, or in some cases more easily, with scripting languages. CSVTest begins to win out when things get a bit more complex, however. This script generates 52 CSV records representing four hands in a game of Bridge:

```
<csvt>
  <gen>
    <shuffle>
      <product>
        <rows values="C,D,H,S" random="no"/>
        <rows values="2,3,4,5,6,7,8,9,10,J,Q,K,A" random="no"/>
      </product>
    </shuffle>
  </gen>
</csvt>
```
