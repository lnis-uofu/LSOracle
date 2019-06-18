#!/usr/bin/env python
# -*- coding: utf-8 -*-

import unittest
from clean_drat import *

class TestDratClean(unittest.TestCase):

  def test_empty(self):
      self.assertEqual(fix_line('0'), '0')

  def test_id(self):
      self.assertEqual(fix_line('0 2343'), '0')

  def test_id2(self):
      self.assertEqual(fix_line('2 3 -4 0 2343'), '2 3 -4 0')

  def test_id_d(self):
      self.assertEqual(fix_line('d 0 2343'), 'd 0')

  def test_id_d2(self):
      self.assertEqual(fix_line('d 5 -1 2 0 2343'), 'd 5 -1 2 0')

if __name__ == '__main__':
    unittest.main()
