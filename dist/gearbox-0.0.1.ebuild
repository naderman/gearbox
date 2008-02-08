# Copyright 1999-2007 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit cmake-utils

SLOT="0"
LICENSE="GPL-2"
KEYWORDS="~x86 ~amd64"
DESCRIPTION="A collection of libraries for robotics, including hardware drivers and algorithms."
SRC_URI="/${P}.tar.bz2"
HOMEPAGE="http://gearbox.sourceforge.net"
IUSE="doc basic gbxadvanced urg_nz"
DEPEND=">=dev-util/cmake-2.4
		doc? (app-doc/doxygen)"
RDEPEND=${DEPEND}

S=${WORKDIR}/${PN}

src_compile()
{
	local mycmakeargs="`cmake-utils_use_enable basic LIB_BASIC`\
						`cmake-utils_use_enable gbxadvanced LIB_GBXADVANCED`\
						`cmake-utils_use_enable urg_nz LIB_URG_NZ`"
	cmake-utils_src_compile
	if use doc;
	then
		cd doc
		doxygen doxyfile || die "Error building documentation"
	fi
}

src_install ()
{
	cmake-utils_src_install
	dodoc LICENSE
	use doc && dohtml -r doc/html/*
}