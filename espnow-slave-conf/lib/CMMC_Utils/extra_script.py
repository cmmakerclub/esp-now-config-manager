from SCons.Script import DefaultEnvironment
import glob, shutil, os

Import("env")

# env = DefaultEnvironment()
# uncomment line below to see environment variables
# print env.Dump()
# print ARGUMENTS

def before_upload(source, target, env):
    print "before_upload"

def after_upload(source, target, env):
    print "after_upload"
    shutil.rmtree("lib-dev/CMMC_Utils")

def before_build(source, target, env):
    print "before_build"

if not os.path.exists("lib-dev/CMMC_Utils"):
    os.makedirs("lib-dev/CMMC_Utils")
for file in glob.iglob('src/*.*'):
    print 'Copied file %s' % (file)
    shutil.copy2(file, "lib-dev/CMMC_Utils/")

print "Current build targets", map(str, BUILD_TARGETS)

env.AddPreAction("upload", before_upload)
env.AddPostAction("upload", after_upload)
env.AddPreAction("buildprog", before_build)