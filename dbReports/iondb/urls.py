# Copyright (C) 2010 Ion Torrent Systems, Inc. All Rights Reserved

from django.conf import settings
from django.conf.urls.defaults import *
from django.http import HttpResponsePermanentRedirect

# Uncomment the next two lines to enable the admin:
from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns(
    r'',
    (r'^rundb/', include('iondb.rundb.urls')),
    (r'^admin/doc/', include('django.contrib.admindocs.urls')),
    (r'^admin/manage/$', 'iondb.rundb.admin.manage'),
    (r'^admin/network/$', 'iondb.rundb.admin.network'),
    (r'^admin/network/check$', 'iondb.rundb.admin.how_am_i'),
    (r'^admin/network/external_ip/$', 'iondb.rundb.admin.fetch_remote_content', {"url": settings.EXTERNAL_IP_URL}),
    (r'^admin/network/how_is/(?P<host>[\w\.]+):(?P<port>\d+)/feeling$', 'iondb.rundb.admin.how_are_you'),
    (r'^admin/update/$', 'iondb.rundb.admin.update'),
    (r'^admin/update/check/$', 'iondb.rundb.admin.run_update_check'),
    (r'^admin/updateOneTouch/$', 'iondb.rundb.admin.updateOneTouch'),
    (r'^admin/updateOneTouch/ot_log$', 'iondb.rundb.admin.ot_log'),
    (r'^admin/update/install_log$', 'iondb.rundb.admin.install_log'),
    (r'^admin/update/install_lock$', 'iondb.rundb.admin.install_lock'),
    (r'^admin/', include(admin.site.urls)),
)

if settings.TEST_INSTALL:
    from os import path
    urlpatterns.extend(patterns(
            '',
            (r'^site_media/(?P<path>.*)$',
             'django.views.static.serve',
             {'document_root':settings.MEDIA_ROOT}),
            (r'^testreports/(?P<path>.*)$',
             'django.views.static.serve',
             {'document_root':path.join(path.dirname(settings.MEDIA_ROOT),
                                                     "testreports")})))
