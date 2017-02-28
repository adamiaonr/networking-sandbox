import struct
import binascii
import collections

class MetaFrame:

    def __init__(self):

        # order of fields within the packet is important
        self.frame = collections.OrderedDict()

        self.frame['header']    = collections.OrderedDict()
        self.frame['data']      = collections.OrderedDict()
        self.frame['footer']    = collections.OrderedDict()

        # important for unpacking (must be set on the subclasses)
        self.hdr_size = 0
        self.ftr_size = 0

    def get_attr(self, part, attr):

        try:
            return self.frame[part][attr]['value']
        except KeyError:
            return None

    def set_attr(self, part, attr, value, size = None):
        self.frame[part][attr]['value'] = value

        if size:
            self.frame[part][attr]['size'] = size

    def pack(self, parts = ['header', 'data', 'footer'], field_exceptions = []):

        values = []
        format_str = '! '

        # pack each field into a big endian byte array, using the respective 
        # type
        for part in parts:
            for field in self.frame[part]:

                if field in field_exceptions:
                    continue

                values.append(self.frame[part][field]['value'])
                format_str += ('%d%s ' % (self.frame[part][field]['size'], self.frame[part][field]['type']))

        return struct.pack(format_str.rstrip(' '), *values)

    def unpack_hdr(self, raw_frame, field_exceptions = []):

        # build the format string
        format_str = '! '

        for field in self.frame['header']:

            if field in field_exceptions:
                continue

            format_str += ('%d%s ' % (self.frame['header'][field]['size'], self.frame['header'][field]['type']))

        # unpack the data into a tuple made of frame's fields
        unpacked_hdr = struct.unpack(format_str.rstrip(' '), raw_frame[:self.hdr_size])
        print("MetaFrame::unpack_hdr() : [INFO] unpacked %s" % (str(unpacked_hdr)))

        # finally, re-set the frame's fields, extracted from the unpacked tuple 
        i = 0
        for field in self.frame['header']:
            self.set_attr('header', field, unpacked_hdr[i])
            i += 1

        return 0

    def unpack_data(self, raw_frame, field_exceptions = []):

        start = self.hdr_size
        end = len(raw_frame)
        if start >= end:
            print("MetaFrame::unpack_data() : [ERROR] malformed packet (start > end)")
            return -1
        else:
            print("MetaFrame::unpack_data() : [INFO] payload size : %d" % (end - start))

        # build the format string
        format_str = '! '

        for field in self.frame['data']:

            if field in field_exceptions:
                continue

            format_str += ('%d%s ' % ((end - start), self.frame['data'][field]['type']))

        # unpack the data into a tuple made of frame's fields
        unpacked_data = struct.unpack(format_str.rstrip(' '), raw_frame[start:end])
        print("MetaFrame::unpack_data() : [INFO] unpacked %s" % (str(unpacked_data)))

        # finally, re-set the frame's fields, extracted from the unpacked tuple 
        i = 0
        for field in self.frame['data']:
            self.set_attr('data', field, unpacked_data[i])
            i += 1

        return 0

    def unpack(self, raw_frame, field_exceptions = []):

        self.unpack_hdr(raw_frame, field_exceptions)
        self.unpack_data(raw_frame, field_exceptions)

        return 0