#!/bin/bash
# rsync script
rsync --exclude='.DS_Store' -a /Users/vinnie/Desktop/Rasberry\ Pi/pumphouse/* vinthewrench@pi5:projects/pumphouse
