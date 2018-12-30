<xsl:transform version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:check="http://check.sourceforge.net/ns">

<xsl:output omit-xml-declaration="yes" indent="yes" doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd" method="xml"/>

<xsl:variable name="baseUri" select="/config/baseSiteUri"/>
<xsl:variable name="checkFile" select="/config/checkFile"/>
<xsl:variable name="checkFolder" select="/config/checkFolder"/>
<xsl:template match="/">
    <!-- ok, first read the configuration -->
    <html><head>
        <title>Automated test result</title>
	<xsl:element name="LINK">
		<xsl:attribute name="rel">stylesheet</xsl:attribute>
		<xsl:attribute name="type">text/css</xsl:attribute>
		<xsl:attribute name="href"><xsl:value-of select="$baseUri"/>checkReport.css</xsl:attribute>
	</xsl:element></head><body>
    <xsl:choose>
    <xsl:when test="/config/checkFile">
      <xsl:for-each select="document(concat($checkFolder,'/',$checkFile))">
        <xsl:call-template name="Header"/>
      </xsl:for-each>
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name="GenericHeader"/>
    </xsl:otherwise>
    </xsl:choose>
    <xsl:for-each select="document(concat($checkFolder,'/.automenu.xml'))/menu">
        <xsl:call-template name="menuEntry"/>
    </xsl:for-each>
    <xsl:if test="/config/checkFile">
    <xsl:for-each select="document(concat($checkFolder,'/',$checkFile))/check:testsuites">
        <xsl:call-template name="Results"/>
    </xsl:for-each>
    </xsl:if>
    </body></html>
</xsl:template>

<xsl:template name="showErrorIcon">
    <xsl:element name="img">
        <xsl:attribute name="alt">error</xsl:attribute>
	<xsl:attribute name="src"><xsl:value-of select="$baseUri"/>error.png</xsl:attribute>
    </xsl:element>
</xsl:template>
<xsl:template name="showFailureIcon">
    <xsl:element name="img">
        <xsl:attribute name="alt">failure</xsl:attribute>
	<xsl:attribute name="src"><xsl:value-of select="$baseUri"/>failure.png</xsl:attribute>
    </xsl:element>
</xsl:template>
<xsl:template name="showSuccessIcon">
    <xsl:element name="img">
        <xsl:attribute name="alt">success</xsl:attribute>
	<xsl:attribute name="src"><xsl:value-of select="$baseUri"/>success.png</xsl:attribute>
    </xsl:element>
</xsl:template>

<xsl:template name="menuEntry">
    <div class="testMenu">
    <xsl:if test="parentMenu">
        <div class="parentMenu">
            <a href="../.index.html"><xsl:value-of select="parentMenu"/></a>
	</div>
    </xsl:if>
<xsl:for-each select="entry">
    <div class="testMenuEntry">

    <div class="menuEntryStatus">
    <xsl:choose>
    	<xsl:when test="@error != '0'">
	    <xsl:call-template name="showErrorIcon"/>
	</xsl:when>
	<xsl:when test="@failure != '0'">
	    <xsl:call-template name="showFailureIcon"/>
	</xsl:when>
	<xsl:otherwise>
	    <xsl:call-template name="showSuccessIcon"/>
	</xsl:otherwise>
    </xsl:choose></div>
    <div class="menuEntryWeb"><xsl:element name="a">
    	<xsl:attribute name="href"><xsl:value-of select="."/><xsl:text>.html</xsl:text></xsl:attribute>
	<xsl:value-of select="."/>
    </xsl:element></div>
    <div class="menuEntryOut"><xsl:element name="a">
    	<xsl:attribute name="href"><xsl:value-of select="."/><xsl:text>.out</xsl:text></xsl:attribute>
		<xsl:element name="img">
			<xsl:attribute name="alt">text log</xsl:attribute>
			<xsl:attribute name="src"><xsl:value-of select="$baseUri"/>txtlog.png</xsl:attribute>
		</xsl:element>
    </xsl:element></div></div>
</xsl:for-each>
<xsl:for-each select="subDirectory">
    <div class="testMenuEntry">
    <xsl:element name="a">
        <xsl:attribute name="href"><xsl:value-of select="@name"/>/.index.html</xsl:attribute>
        <xsl:value-of select="@name"/>
    </xsl:element>
    </div>
</xsl:for-each>
</div>
</xsl:template>

<xsl:template name="GenericHeader">
        <!-- date of test -->
        <div class="header">
        <div class="testLogo"><a href="http://sourceforge.net"><img
src="http://sflogo.sourceforge.net/sflogo.php?group_id=13833&amp;type=1"
width="88" height="31" border="0" alt="SourceForge.net Logo" /></a>
        <xsl:element name="img">
		<xsl:attribute name="src"><xsl:value-of select="$baseUri"/>crossfire-logo-unit.png</xsl:attribute>
		<xsl:attribute name="alt"><xsl:text>Crossfire testing logo</xsl:text></xsl:attribute>
        </xsl:element>
        </div>
        </div>
</xsl:template>

<xsl:template name="Header">
        <!-- date of test -->
        <div class="header">
        <div class="testLogo"><a href="http://sourceforge.net"><img
src="http://sflogo.sourceforge.net/sflogo.php?group_id=13833&amp;type=1"
width="88" height="31" border="0" alt="SourceForge.net Logo" /></a>
        <xsl:element name="img">
        	<xsl:choose>
        	<xsl:when test="check:testsuites/check:suite/check:test[@result='error' or @result='failure']">
        		<xsl:attribute name="src"><xsl:value-of select="$baseUri"/>crossfire-logo-unit-failed.png</xsl:attribute>
        		<xsl:attribute name="alt">Logo for failing test</xsl:attribute>
        	</xsl:when>
        	<xsl:otherwise>
        		<xsl:attribute name="src"><xsl:value-of select="$baseUri"/>crossfire-logo-unit-success.png</xsl:attribute>
        		<xsl:attribute name="alt">Logo for successful test</xsl:attribute>
        	</xsl:otherwise>
        	</xsl:choose>
        </xsl:element>
        </div>
        <div class="runDate"><xsl:text>This test was run on </xsl:text><xsl:value-of select="check:testsuites/check:datetime"/></div>
        <div class="runDuration"><xsl:text>in </xsl:text><xsl:value-of select="check:testsuites/check:duration"/><xsl:text> seconds</xsl:text></div>
        </div>
</xsl:template>


<xsl:template name="Results">
    <div class="testRoot">
        <div class="resume">
        <!-- show a quick link of problem tests -->
        <xsl:if test="check:suite/check:test[@result='error' or @result='failure']">
        <div class="failedTestList">
            <xsl:text>The following tests have problems: </xsl:text>
            <xsl:for-each select="check:suite/check:test[@result='error']">
                <xsl:element name="a">
                    <xsl:attribute name="href">#test_<xsl:value-of select="check:id"/></xsl:attribute>
                    <xsl:attribute name="class">errorTestLink</xsl:attribute>
		    <xsl:call-template name="showErrorIcon"/>
                    <xsl:value-of select="check:id"/>
                </xsl:element>
            </xsl:for-each>
            <xsl:for-each select="check:suite/check:test[@result='failure']">
                <xsl:element name="a">
                    <xsl:attribute name="href">#test_<xsl:value-of select="check:id"/></xsl:attribute>
                    <xsl:attribute name="class">failureTestLink</xsl:attribute>
		    <xsl:call-template name="showFailureIcon"/>
                    <xsl:value-of select="check:id"/>
                </xsl:element>
            </xsl:for-each>
        </div></xsl:if>
        <!-- show the list of suites -->
        <div class="suiteList">
            <xsl:text>This test contains the following suites: </xsl:text>
            <xsl:for-each select="check:suite">
                <xsl:element name="a">
                    <xsl:attribute name="href">#suite_<xsl:value-of select="check:title"/></xsl:attribute>
                    <xsl:attribute name="class">suiteLink</xsl:attribute>
                    <xsl:value-of select="check:title"/>
                </xsl:element>
            </xsl:for-each>
        </div>
        </div>
        <!-- now apply the suites templates -->
        <xsl:for-each select="check:suite">
            <xsl:call-template name="suiteOutput"/>
	</xsl:for-each>
    </div>
</xsl:template>
<xsl:template name="suiteOutput">
    <div class="suiteBox">
        <xsl:element name="a">
            <xsl:attribute name="name">suite_<xsl:value-of select="check:title"/></xsl:attribute>
        </xsl:element>
        <div class="suiteHead">
            <xsl:text>Results for suite </xsl:text><xsl:value-of select="check:title"/><xsl:text>:</xsl:text>
        </div>
        <div class="suiteContent">
            <xsl:for-each select="check:test">
                <xsl:call-template name="testOutput"/>
            </xsl:for-each>
        </div>
        <div class="suiteTail"></div>
    </div>
</xsl:template>
<xsl:template name="testOutput">
    <xsl:variable name="theClass">
    <xsl:choose>
        <xsl:when test="@result = 'failure'">
             <xsl:text>testStatusFailure</xsl:text>
        </xsl:when>
        <xsl:when test="@result = 'error'">
             <xsl:text>testStatusError</xsl:text>
        </xsl:when>
        <xsl:otherwise>
             <xsl:text>testStatusSuccess</xsl:text>
        </xsl:otherwise>
    </xsl:choose>
    </xsl:variable>
    <xsl:element name="div">
        <xsl:attribute name="class">testBox <xsl:value-of select="$theClass"/></xsl:attribute>
        <div class="testHead">
	    <xsl:choose>
    		<xsl:when test="@result = 'error'">
		    <xsl:call-template name="showErrorIcon"/>
		</xsl:when>
	    	<xsl:when test="@result = 'failure'">
		    <xsl:call-template name="showFailureIcon"/>
		</xsl:when>
		<xsl:otherwise>
		    <xsl:call-template name="showSuccessIcon"/>
		</xsl:otherwise>
	    </xsl:choose>
            <xsl:text>Test </xsl:text><xsl:value-of select="check:id"/>
        </div>
        <xsl:if test="@result = 'failure' or @result = 'error'">
        <div class="testContent">
        <xsl:choose>
            <xsl:when test="@result = 'failure'">
                <xsl:text>Failed</xsl:text>
            </xsl:when>
            <xsl:when test="@result = 'error'">
                <xsl:text>Problems running</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>Success</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <br/>
        <xsl:value-of select="check:message"/>
        <br/>
        <xsl:value-of select="check:fn"/>
        </div></xsl:if>
    </xsl:element>
</xsl:template>
</xsl:transform>
