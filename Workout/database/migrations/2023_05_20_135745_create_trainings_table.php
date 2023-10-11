<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

return new class extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {

        Schema::create('trainings', function (Blueprint $table) {
            $table->id();
            $table->string('id_card')->unique();
            $table->string('service_number')->unique();
            $table->string('rank');
            $table->string('name');
            $table->string('age');
            $table->string('gender');
            $table->string('height');
            $table->string('weight');
            $table->string('push_up')->nullable();
            $table->string('pull_up')->nullable();
            $table->string('abs')->nullable();
            $table->string('running')->nullable();
            $table->timestamps();
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('trainings');
    }
};
