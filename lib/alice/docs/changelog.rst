Change Log
==========

v0.4 (not yet released)
-----------------------

* General commands: ``help --docs`` to generate documentation of all commands
  `#10 <https://github.com/msoeken/alice/pull/10>`_

v0.3 (July 22, 2018)
--------------------

* Throw and catch errors in `read`.

* General commands: ``ps --all`` to show statistics of all store entries
  `#5 <https://github.com/msoeken/alice/pull/5>`_

* General commands: Read multiple files in ``read``
  `#6 <https://github.com/msoeken/alice/pull/6>`_

* Support for default store option (enabled via setting ``ALICE_SETTINGS_WITH_DEFAULT_OPTION``)
  `#7 <https://github.com/msoeken/alice/pull/7>`_

* General commands: ``store --pop`` to remove current store element
  `#8 <https://github.com/msoeken/alice/pull/8>`_

* Automatic ``to_<tag>`` in Python interface as shortcut for ``write_<tag>(log=True)["contents"]``
  `#9 <https://github.com/msoeken/alice/pull/9>`_

v0.2 (May 7, 2018)
------------------

* Validators: ``ExistingFileWordExp``

* C library interface
  `#1 <https://github.com/msoeken/alice/pull/1>`_

* General commands: ``write_<format> --log`` to write file contents to log
  `#2 <https://github.com/msoeken/alice/pull/2>`_

v0.1 (January 11, 2018)
-----------------------

* Initial release

* General commands: ``alias``, ``convert``, ``current``, ``help``, ``print``, ``ps``, ``quit``, ``set``, ``show``, ``store``

* Shell application command line flags: ``--command``, ``--filename``, ``--echo``, ``--counter``, ``--interactive``, ``--log``

* Macro API: ``ALICE_MAIN``, ``ALICE_ADD_STORE``, ``ALICE_DESCRIBE_STORE``, ``ALICE_PRINT_STORE``, ``ALICE_PRINT_STORE_STATISTICS``, ``ALICE_LOG_STORE_STATISTICS``, ``ALICE_CONVERT``, ``ALICE_SHOW``, ``ALICE_STORE_HTML``, ``ALICE_ADD_COMAND``, ``ALICE_COMMAND``, ``ALICE_READ_FILE``, ``ALICE_WRITE_FILE``, ``ALICE_ADD_FILE_TYPE``, ``ALICE_ADD_FILE_TYPE_READ_ONLY``, ``ALICE_ADD_FILE_TYPE_WRITE_ONLY``

* Python special features: ``__repr__`` (from ``print``), and ``_repr_html_`` (from ``html_repr``)
