// https://jenkins.io/doc/book/pipeline/jenkinsfile/
// Scripted pipeline (not declarative)
pipeline {
  options {
    buildDiscarder(logRotator(numToKeepStr: '10'))
  }
  agent {
    docker { 
      image 'mauriciojost/arduino-ci:platformio-3.5.3-0.2.0' 
    }
  }
  stages {
    stage('Build') {
      steps {
        script {
          sshagent(['bitbucket_key']) {
            wrap([$class: 'AnsiColorBuildWrapper', 'colorMapName': 'xterm']) {
              sh 'export GIT_COMMITTER_NAME=jenkinsbot && export GIT_COMMITTER_EMAIL=mauriciojostx@gmail.com && set && ./pull_dependencies'
              sh './upload -p profiles/build.prof'
            }
          }
        }
      }
    }
    stage('Test') {
      steps {
        wrap([$class: 'AnsiColorBuildWrapper', 'colorMapName': 'xterm']) {
          sh './launch_tests'
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
          sh './upload -p profiles/generic.prof -e'
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
      archiveArtifacts artifacts: 'firmware-*.bin', fingerprint: true;
      archiveArtifacts artifacts: 'gcovr*', fingerprint: true;
    }  
  }
}
