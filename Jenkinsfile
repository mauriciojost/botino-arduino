// https://jenkins.io/doc/book/pipeline/jenkinsfile/
// Scripted pipeline (not declarative)
pipeline {
  options {
    buildDiscarder(logRotator(numToKeepStr: '10'))
  }
  agent {
    docker { 
      image 'mauriciojost/arduino-ci:platformio-3.5.3-0.1.0' 
    }
  }
  stages {
    stage('Build') {
      steps {
        script {
          sshagent(['bitbucket_key']) {
            wrap([$class: 'AnsiColorBuildWrapper', 'colorMapName': 'xterm']) {
              sh 'export GIT_COMMITTER_NAME=jenkinsbot && export GIT_COMMITTER_EMAIL=mauriciojostx@gmail.com && set && ./pull_dependencies'
              sh 'PLATFORMIO_BUILD_FLAGS="-D PROJ_VERSION=test `cat profiles/build.prof | grep -v '^\#'`" platformio run'
            }
          }
        }
      }
    }
    stage('Test') {
      steps {
        wrap([$class: 'AnsiColorBuildWrapper', 'colorMapName': 'xterm']) {
          sh './launch_tests profiles/test.prof'
        }
      }
    }
    stage('Simulate') {
      steps {
        wrap([$class: 'AnsiColorBuildWrapper', 'colorMapName': 'xterm']) {
          sh './simulate profiles/simulate.prof 1 10' 
        }
      }
    }
    stage('Artifact') {
      steps {
        wrap([$class: 'AnsiColorBuildWrapper', 'colorMapName': 'xterm']) {
          sh 'PLATFORMIO_BUILD_FLAGS="-D PROJ_VERSION=`git rev-parse --short HEAD` `cat profiles/generic.prof | grep -v '^\#'``" platformio run'
          sh 'export commitid=`git rev-parse HEAD` && cp .pioenvs/main/firmware.bin firmware-$commitid.bin'
        }
      }
    }
  }
  post {  
    failure {  
      emailext body: "<b>[JENKINS] Failure</b>Project: ${env.JOB_NAME} <br>Build Number: ${env.BUILD_NUMBER} <br> Build URL: ${env.BUILD_URL}", from: '', mimeType: 'text/html', replyTo: '', subject: "ERROR CI: ${env.JOB_NAME}", to: "mauriciojostx@gmail.com", attachLog: true, compressLog: false;
    }  
    success {  
      emailext body: "<b>[JENKINS] Success</b>Project: ${env.JOB_NAME} <br>Build Number: ${env.BUILD_NUMBER} <br> Build URL: ${env.BUILD_URL}", from: '', mimeType: 'text/html', replyTo: '', subject: "SUCCESS CI: ${env.JOB_NAME}", to: "mauriciojostx@gmail.com", attachLog: false, compressLog: false;
      archiveArtifacts artifacts: 'firmware-*.bin', fingerprint: true
    }  
  }
}
