using System;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace OmGui
{
    class ListViewColumnSorter : IComparer
    {
        private int ColumnToSort;
        private SortOrder OrderOfSort;
        private CaseInsensitiveComparer ObjectCompare;

        public ListViewColumnSorter()
        {
            ColumnToSort = 0;
            OrderOfSort = SortOrder.None;
            ObjectCompare = new CaseInsensitiveComparer();
        }

        public int Compare(object x, object y)
        {
            int compareResult;
            ListViewItem listViewItemX, listViewItemY;

            listViewItemX = (ListViewItem)x;
            listViewItemY = (ListViewItem)y;

            compareResult = ObjectCompare.Compare(listViewItemX.SubItems[ColumnToSort].Text, listViewItemY.SubItems[ColumnToSort].Text);

            if (OrderOfSort == SortOrder.Ascending)
                return compareResult;
            else if (OrderOfSort == SortOrder.Descending)
                return (-compareResult);
            else
                return 0;
        }

        public int SortColumn
        {
            get
            {
                return ColumnToSort;
            }
            set
            {
                ColumnToSort = value;
            }
        }

        public SortOrder Order
        {
            get
            {
                return OrderOfSort;
            }
            set
            {
                OrderOfSort = value;
            }
        }
    }
}