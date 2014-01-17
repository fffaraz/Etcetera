#!/usr/bin/env python
import random

def get_random_useragent():
    base_agent = "Mozilla/%.1f (Windows; U; Windows NT 5.1; en-US; rv:%.1f.%.1f) Gecko/%d0%d Firefox/%.1f.%.1f"
    return base_agent % ((random.random() + 5),
                         (random.random() + random.randint(1, 8)), random.random(),
                         random.randint(2000, 2100), random.randint(92215, 99999),
                         (random.random() + random.randint(3, 9)), random.random())
