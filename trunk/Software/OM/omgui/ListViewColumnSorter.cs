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
        private bool sortByDate;

        public ListViewColumnSorter(bool sortByDate)
        {
            this.sortByDate = sortByDate;
            ColumnToSort = 0;
            OrderOfSort = SortOrder.None;
            ObjectCompare = new CaseInsensitiveComparer();
        }

        public int Compare(object x, object y)
        {
            if (sortByDate)
            {
                int returnVal;
                // Determine whether the type being compared is a date type.
                try
                {
                    System.Globalization.CultureInfo provider = System.Globalization.CultureInfo.CurrentCulture;
                    string format = "dd/MM/yy hh:mm:ss";
                    string s = ((ListViewItem)x).SubItems[3].Text;
                    string s2 = ((ListViewItem)y).SubItems[3].Text;
                    // Parse the two objects passed as a parameter as a DateTime.
                    System.DateTime firstDate =
                            DateTime.ParseExact(s, format, provider);
                    System.DateTime secondDate =
                            DateTime.ParseExact(s2, format, provider);
                    // Compare the two dates.
                    returnVal = DateTime.Compare(firstDate, secondDate);
                }
                // If neither compared object has a valid date format, compare
                // as a string.
                catch(FormatException e)
                {
                    Console.WriteLine(e.Message);
                    // Compare the two items as a string.
                    returnVal = String.Compare(((ListViewItem)x).SubItems[3].Text,
                                ((ListViewItem)y).SubItems[3].Text);
                }
                // Determine whether the sort order is descending.
                if (Order == SortOrder.Descending)
                    // Invert the value returned by String.Compare.
                    returnVal *= -1;
                return returnVal;
            }
            else
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