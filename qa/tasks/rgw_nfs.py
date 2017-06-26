"""
RGWFile Unit Suite Task (RGW NFS)
"""
from cStringIO import StringIO
import logging

from teuthology import misc
from teuthology.exceptions import ConfigError
from teuthology.task import Task
from teuthology.orchestra import run

log = logging.getLogger(__name__)


class RGWFile(Task):
    """
    Runs a subset of librgw_file_* unit tests against a minimal RGW cluster.
    The unit tests, like radosgw-admin, are gateway instances, so they are
    run on an RGW node.

    Complete example:

        tasks:
        - install:
        - ceph:
        - rgw: [client.0]
        - rgw_file:
            client: client.0
            bucket_name: nfsroot

    """
    def __init__(self, ctx, config):
        super(RGWFile, self).__init__(ctx, config)
        self.log = log
        log.info('RGWFile __init__')

    def setup(self):
        super(RGWFile, self).setup()
        config = self.config
        log.info('RGWFile setup')
        log.debug('config is: %r', config)
	ctx = self.ctx
        log.debug('ctx is: %r', ctx)
        self.client = config.keys()['client']
        self.bucket_name = config.keys()['bucket_name']
        (remote,) = ctx.cluster.only(client).remotes.iterkeys()
        self.remote = remote

    def begin(self):
        super(RGWFile, self).begin()
        log.info('RGWFile begin')
        remote = self.remote

        # testing
        #remote.run(args=['sleep', '15'], stdout=StringIO())

        # create run, nfsns
        remote.run(args=['ceph_test_librgw_file_nfsns', '--dirs1', '--setattr',
                   '--writef', '--hier1', '--create'])

        # get/put run
        bn = self.bucket_name
        remote.run(args=['ceph_test_librgw_file_gp', '--bn="%s" % (bn)',
                         '--get', '--put', '--delete'])

        # delete run, nfsns
        remote.run(args=['ceph_test_librgw_file_nfsns', '--dirs1'
                         '--setattr', '--writef', '--delete'])

    def end(self):
        # do it
        super(RGWFile, self).end()

task = RGWFile


