# CF_IS_XSLT_COMPLIANT(progpath,ACTION-IF-FOUND, ACTION_IF_NOT_FOUND)
# check for xslt compliance of a given prog, prog must be a full executable
# execution command, in this command, this substitution will be donne:
# %1  = xml file
# %2  = xsl file
# %3  = html file
#
AC_DEFUN([CF_IS_XSLT_COMPLIANT],[
    cat << \EOF > configtest.xml
<?xml version="1.0" encoding="ISO-8859-1"?>
<tool>
  <field id="prodName">
    <value>HAMMER HG2606</value>
  </field>
  <field id="prodNo">
    <value>32456240</value>
  </field>
  <field id="price">
    <value>$30.00</value>
  </field>
</tool>
EOF
    cat << \EOF > configtest.xsl
<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/">
<html>
<body>
<form method="post" action="edittool.asp">
<h2>Tool Information (edit):</h2>
<table border="0">
<xsl:for-each select="tool/field">
<tr>
<td>
<xsl:value-of select="@id"/>
</td>
<td>
<input type="text">
<xsl:attribute name="id">
  <xsl:value-of select="@id" />
</xsl:attribute>
<xsl:attribute name="name">
  <xsl:value-of select="@id" />
</xsl:attribute>
<xsl:attribute name="value">
  <xsl:value-of select="value" />
</xsl:attribute>
</input>
</td>
</tr>
</xsl:for-each>
</table>
<br />
<input type="submit" id="btn_sub" name="btn_sub" value="Submit" />
<input type="reset" id="btn_res" name="btn_res" value="Reset" />
</form>
</body>
</html>
</xsl:template>
</xsl:stylesheet>
EOF
    AC_MSG_CHECKING([xslt compliance of $1])
    to_run=$1
    to_run=${to_run/\%1/configtest.xml}
    to_run=${to_run/\%2/configtest.xsl}
    to_run=${to_run/\%3/configtest.out}
    if AC_TRY_COMMAND([$to_run]);then
    	AC_MSG_RESULT([yes]);
        [$2]
    else
    	AC_MSG_RESULT([no]);
        [$3]
    fi
])

AC_DEFUN([CF_CHECK_XSLT],[
    AC_ARG_WITH(xsltproc,
        [AS_HELP_STRING([--with-xsltproc=path], [specify xslt engine to use for test report generation])],
        [check_xslt_forcedprogfound=$withval])
    if test "x$check_xslt_forcedprogfound" != "x";  then
        AC_PATH_PROG([check_xslt_forcedprogfound],[$check_xslt_forcedprogfound],[notfound])
    fi
    AC_PATH_PROG([check_xslt_xsltprocfound],[xsltproc],[notfound])
    AC_PATH_PROG([check_xslt_sablotronfound],[sabcmd],[notfound])
    xslt_prog="notfound"
    if test "$check_xslt_forcedprogfound" != notfound -a "x$check_xslt_forcedprogfound" != x; then
        xslt_prog="$check_xslt_forcedprogfound"
        CF_IS_XSLT_COMPLIANT([$xslt_prog],[$1=$xslt_prog],[xslt_prog="notfound"])
    fi
    xslt_prog="notfound"
    if test "$check_xslt_xsltprocfound" != notfound -a "$xslt_prog" = notfound; then
        xslt_prog="$check_xslt_xsltprocfound -o %3 %2 %1"
        CF_IS_XSLT_COMPLIANT([$xslt_prog],[$1=$xslt_prog],[xslt_prog="notfound"])
    fi
    if test "$check_xslt_sablotronfound" != notfound -a "$xslt_prog" = notfound; then
        xslt_prog="$check_xslt_sablotronfound %2 %1 %3"
        CF_IS_XSLT_COMPLIANT([$xslt_prog],[$1=$xslt_prog],[xslt_prog="notfound"])
    fi
])
