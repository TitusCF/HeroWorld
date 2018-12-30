<?xml version="1.0" encoding="ISO-8859-1" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html"/>
<xsl:template match="ARCHES">

<table border="1" bgcolor="#fff">
<tr bgcolor="#aaf" align ='center'>
<td>Object</td>
<td>Name</td>
<td>Face</td>
<td>level</td>
<td>wc</td>
<td>ac</td>
<td>dam</td>
<td>exp</td>
<td>speed</td>
<td>str</td>
<td>dex</td>
<td>con</td>
<td>int</td>
<td>wis</td>
<td>pow</td>
<td>cha</td>
<td>resist_physical</td>
<td>resist_poison</td>
<td>can_use_shield</td>
<td>can_use_armour</td>
<td>can_use_weapon</td>
<td>can_see_in_dark</td>
</tr>

<xsl:for-each select="arch">
<tr bgcolor='#ccf' align='center'>
<td><xsl:value-of select="object"/></td>
<td><xsl:value-of select="name"/></td>
<td><xsl:value-of select="face"/></td>
<td><xsl:value-of select="level"/></td>
<td><xsl:value-of select="wc"/></td>
<td><xsl:value-of select="ac"/></td>
<td><xsl:value-of select="dam"/></td>
<td><xsl:value-of select="exp"/></td>
<td><xsl:value-of select="speed"/></td>
<td><xsl:value-of select="str"/></td>
<td><xsl:value-of select="dex"/></td>
<td><xsl:value-of select="con"/></td>
<td><xsl:value-of select="int"/></td>
<td><xsl:value-of select="wis"/></td>
<td><xsl:value-of select="pow"/></td>
<td><xsl:value-of select="cha"/></td>
<td><xsl:value-of select="resist_physical"/></td>
<td><xsl:value-of select="resist_poison"/></td>
<td><xsl:value-of select="can_use_shield"/></td>
<td><xsl:value-of select="can_use_armour"/></td>
<td><xsl:value-of select="can_use_weapon"/></td>
<td><xsl:value-of select="can_see_in_dark"/></td>

</tr>
</xsl:for-each>
</table>
</xsl:template>
</xsl:stylesheet>
