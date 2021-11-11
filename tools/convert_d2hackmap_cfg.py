#!/usr/bin/env python3

import codecs
import csv


item_map = {}


def conver_item_id(item_id):
    if item_id == '0+':
        return ''
    iid = int(item_id)
    if iid > 2000:
        return iid - 2001 + 508
    if iid > 1000:
        return iid - 1001 + 306
    return iid - 1


def convert_num(num):
    return int(num) - 1


def process_ranges(s, fn, find_item_name):
    if s == '0+':
        return '', ''
    global item_map
    res = ''
    res2 = []
    for v in s.split(','):
        v2 = v.split('-')
        low = fn(int(v2[0]))
        if len(v2) > 1:
            high = fn(int(v2[1]))
            res2.append('%d-%d' % (low, high))
        else:
            high = low
            res2.append(str(low))
        if find_item_name:
            for n in range(low, high + 1):
                res = res + ', ' + item_map[n]
    return res, ','.join(res2)


if __name__ == '__main__':
    # d2hackmap.cfg is encoded in GBK for CN version, please convert it to utf-8 before reading here
    with codecs.open('../doc/ItemDesc.csv', 'r', 'utf8') as f:
        reader = csv.reader(f)
        for row in reader:
            if row[0][0] < '0' or row[0][0] > '9':
                continue
            item_map[int(row[0])] = row[2]
    with codecs.open('d2hackmap.cfg', 'r', 'utf8') as f:
        for line in f:
            line_sep = line.split('//')
            sl = line_sep[0].strip().split(':')
            if len(line_sep) > 1:
                comment = line_sep[1].strip()
            else:
                comment = None
            if len(sl) != 2:
                continue
            keys_pre = sl[0].strip().split('[')
            if keys_pre[0] != 'Item Colours':
                continue
            key_indices = []
            if len(keys_pre) > 1:
                key_indices = [n.rstrip(']') for n in keys_pre[1:]]
            ind_count = len(key_indices)
            if ind_count == 0:
                continue
            values = sl[1].strip().split(',')
            if len(values) > 1 and values[1].strip() != '-1':
                value = 3
            else:
                value = 0
            indstr, indrange = process_ranges(key_indices[0], conver_item_id, True)
            ind_arr = [indrange, None, None, None]
            for idx in range(1, ind_count):
                _, indrange = process_ranges(key_indices[idx], convert_num, False)
                ind_arr[idx] = indrange
            if ind_arr[2] == '0,1':
                ind_arr[2] = None
            if comment is not None and len(comment) > 0:
                print(';', comment)
            if indstr is not None and len(indstr) > 0:
                print(';', indstr[2:])
            print(ind_arr[0], end='')
            if ind_arr[1] is not None:
                print('+' + ind_arr[1], end='')
            if ind_arr[3] is not None:
                print('*' + ind_arr[3], end='')
            if ind_arr[2] is not None:
                print('#' + ind_arr[2], end='')
            print(' =', value)
