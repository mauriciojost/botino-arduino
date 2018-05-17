// https://jenkins.io/doc/book/pipeline/jenkinsfile/
// Scripted pipeline (not declarative)
pipeline {
  agent {
    docker { image 'mauriciojost/arduino-ci:latest' }
    sshagent (credentials: ['deploy-dev']) {
      sh 'ssh -o StrictHostKeyChecking=no -l cloudbees 192.168.1.106 uname -a'
    }
  }
  stages {
    stage('Build') {
      steps {
	echo "My branch is: ${env.BRANCH_NAME}"
	sh 'export GIT_COMMITTER_NAME=mjost && export GIT_COMMITTER_EMAIL=mauriciojost@gmail.com && set && ./pull_dependencies'
	sh 'platformio run'
      }
    }
    stage('Test') {
      steps {
	echo "My branch is: ${env.BRANCH_NAME}"
        sh './launch_tests'
      }
    }
  }
}
