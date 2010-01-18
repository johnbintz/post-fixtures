<?php

$builder->category('test')->defer();
$builder->post('My post')
        ->date('2010-02-20')
        ->content('My content')
        ->categories('test,test2')
        ->metadata('field', 'value')
        ->tags('tag1,tag2')
        ->defer();
