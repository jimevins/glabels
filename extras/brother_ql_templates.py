#!/usr/bin/python

"""Generate Glabels templates for Brother QL label printers.

To use:
1. Download DEB Cups Wrapper/LPR Printer driver from e.g. https://www.brother.co.uk/support/ql-820nwb/downloads
2. Unpack with `ar x <packagename>.deb`
3. Unpack data files with `tar xzvf data.tar.gz`
4. Find the PPD file: brother_<model>_printer_en.ppd
5. Specify the ppd file as the first argument on te command line (Sets PPD_FILE variable).
"""

import re
import sys
from collections import defaultdict

PPD_FILE = sys.argv[1] # "brother_ql820nwb_printer_en.ppd"

HEADER = f"""\
<?xml version="1.0"?>

<Glabels-templates>

<!--
   *********************************************************************
   *********************************************************************
   Labels for the Brother QL printers

   Generated using media sizes and measurements from:
   {PPD_FILE}

   *********************************************************************
   *********************************************************************
-->

"""

FOOTER = """
</Glabels-templates>
"""

TEMPLATE = """
<Template brand='Brother' part='{name}' size='Other' width='{x}' height='{y}' _description='{name}'>
    <Meta category='label'/>
    <Meta category='mail'/>
    <Label-rectangle id='0' width='{x}' height='{y}' round='4pt' x_waste='0' y_waste='0'>
      <Markup-rect x1='{x1}' y1='{y1}' w='{w}' h='{h}' r='0' />
      <Layout nx='1' ny='1' x0='0' y0='0' dx='0' dy='0'/>
    </Label-rectangle>
  </Template>
 """

class MakeLabels():
    def __init__(self):
        self.labels = defaultdict(dict)
        self.ppd = None

        self.run()

    def split_ppd(self):
        for line in self.ppd:
            match = re.match(r'\*(\w+) (.+):.*"(.*)"', line)
            if match:
                if match[1] == "ImageableArea" or match[1] == "PaperDimension":
                    dimensions = [float(x) for x in match[3].split()]
                    self.labels[match[1]].update({match[2]: dimensions})

    def make_xml(self):
        label_xml = []
        try:
            for name, size in self.labels['PaperDimension'].items():
                x, y = size
                x1, y1, w, h = self.labels['ImageableArea'][name]
                # Account for bottom/right margin
                w = w - x1
                h = h - y1
                label_xml.append(TEMPLATE.format(name=name, x=x, y=y, x1=x1, y1=y1, w=w, h=h))
        except KeyError as err:
            print(f"Mis-matched label name: {err}\nCheck {PPD_FILE} - sections: PaperDimension and ImageableArea for mistakes.")
        print(HEADER)
        for label in label_xml:
            print(label)
        print(FOOTER)


    def run(self):
        with open(PPD_FILE) as ppd_f:
            self.ppd = ppd_f.readlines()
        self.split_ppd()
        self.make_xml()

if __name__ == "__main__":
    MakeLabels()
